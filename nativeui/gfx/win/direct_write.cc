// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/gfx/win/direct_write.h"

#include <d2d1.h>
#include <wrl/client.h>

#include "base/debug/alias.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/text.h"
#include "nativeui/state.h"
#include "nativeui/system.h"

namespace nu {

namespace {

inline DWRITE_TEXT_ALIGNMENT ToDWriteType(TextAlign align) {
  switch (align) {
    case TextAlign::Center:
      return DWRITE_TEXT_ALIGNMENT_CENTER;
    case TextAlign::End:
      return DWRITE_TEXT_ALIGNMENT_TRAILING;
    case TextAlign::Start:
    default:
      return DWRITE_TEXT_ALIGNMENT_LEADING;
  }
}

inline DWRITE_PARAGRAPH_ALIGNMENT ToDWriteType(TextAlign align, bool) {
  switch (align) {
    case TextAlign::Center:
      return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
    case TextAlign::End:
      return DWRITE_PARAGRAPH_ALIGNMENT_FAR;
    case TextAlign::Start:
    default:
      return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
  }
}

}  // namespace

void CreateDWriteFactory(IDWriteFactory** factory) {
  Microsoft::WRL::ComPtr<IUnknown> factory_unknown;
  HRESULT hr =
      DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                          factory_unknown.GetAddressOf());
  if (FAILED(hr)) {
    base::debug::Alias(&hr);
    CHECK(false);
    return;
  }
  factory_unknown.CopyTo(factory);
}

bool CreateTextLayout(const base::string16& text,
                      const TextFormat& format,
                      IDWriteTextLayout** text_layout) {
  IDWriteFactory* factory = State::GetCurrent()->GetDWriteFactory();

  Font* default_font = System::GetDefaultFont();
  Microsoft::WRL::ComPtr<IDWriteTextFormat> dformat;
  if (FAILED(factory->CreateTextFormat(default_font->GetName16().c_str(),
                                       nullptr,
                                       DWRITE_FONT_WEIGHT_NORMAL,
                                       DWRITE_FONT_STYLE_NORMAL,
                                       DWRITE_FONT_STRETCH_NORMAL,
                                       default_font->GetSize(),
                                       L"",
                                       dformat.GetAddressOf()))) {
    return false;
  }

  dformat->SetTextAlignment(ToDWriteType(format.align));
  dformat->SetParagraphAlignment(ToDWriteType(format.valign, true));
  dformat->SetWordWrapping(format.wrap ? DWRITE_WORD_WRAPPING_WRAP
                                       : DWRITE_WORD_WRAPPING_NO_WRAP);
  if (format.ellipsis) {
    Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
    factory->CreateEllipsisTrimmingSign(dformat.Get(), ellipsis.GetAddressOf());
    DWRITE_TRIMMING trimming = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    dformat->SetTrimming(&trimming, ellipsis.Get());
  }

  return SUCCEEDED(factory->CreateTextLayout(text.c_str(),
                                             static_cast<UINT32>(text.size()),
                                             dformat.Get(), FLT_MAX, FLT_MAX,
                                             text_layout));
}

}  // namespace nu
