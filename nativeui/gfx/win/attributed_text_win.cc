// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/attributed_text_win.h"

#include <string>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/win/screen_win.h"

namespace nu {

AttributedTextImpl::AttributedTextImpl()
    // https://stackoverflow.com/questions/1203087
    : format(Gdiplus::StringFormat::GenericTypographic()) {}

AttributedTextImpl::~AttributedTextImpl() {}

AttributedText::AttributedText(const std::string& text, TextAttributes att)
    : AttributedText(base::UTF8ToWide(text), std::move(att)) {}

AttributedText::AttributedText(std::wstring text, TextAttributes att) {
  text_ = new AttributedTextImpl;
  text_->text = std::move(text);
  SetFormat(att.ToTextFormat());
  SetFont(att.font.get());
  SetColor(att.color);
}

AttributedText::~AttributedText() {
  delete text_;
}

void AttributedText::PlatformUpdateFormat() {
  text_->format.SetAlignment(ToGdi(format_.align));
  text_->format.SetLineAlignment(ToGdi(format_.valign));
  if (format_.ellipsis)
    text_->format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
  int flags = Gdiplus::StringFormatFlagsMeasureTrailingSpaces;
  if (!format_.wrap)
    flags |= Gdiplus::StringFormatFlagsNoWrap;
  text_->format.SetFormatFlags(flags);
}

void AttributedText::PlatformSetFontFor(scoped_refptr<Font> font,
                                        int start, int end) {
  CHECK(start == 0 && end == -1)
      << "AttributedText.SetFontFor does not work on Windows";
  text_->font = std::move(font);
}

void AttributedText::PlatformSetColorFor(Color color, int start, int end) {
  CHECK(start == 0 && end == -1)
      << "AttributedText.SetColorFor does not work on Windows";
  text_->brush.reset(new Gdiplus::SolidBrush(ToGdi(color)));
}

RectF AttributedText::GetBoundsFor(const SizeF& size) const {
  // MeasureString does not take account of the last new line, add a character
  // to make it behave the same with other platforms.
  bool ends_with_newline = text_->text.size() > 0 &&
                           text_->text[text_->text.size() - 1] == L'\n';
  const std::wstring& text = ends_with_newline ? text_->text + L"a"
                                                 : text_->text;

  base::win::ScopedGetDC dc(NULL);
  float scale_factor = GetScalingFactorFromDPI(::GetDeviceCaps(dc, LOGPIXELSX));

  Gdiplus::Graphics graphics(dc);
  // https://stackoverflow.com/questions/1203087/why-is-graphics-measurestring-returning-a-higher-than-expected-number
  graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
  Gdiplus::RectF bounds = ToGdi(RectF(ScaleSize(size, scale_factor)));
  Gdiplus::RectF rect;
  graphics.MeasureString(text.data(), static_cast<int>(text.size()),
                         text_->font->GetNative(), bounds,
                         &rect);
  return RectF(rect.X / scale_factor, rect.Y / scale_factor,
               rect.Width / scale_factor, rect.Height / scale_factor);
}

void AttributedText::SetText(const std::string& text) {
  text_->text = base::UTF8ToWide(text);
}

std::string AttributedText::GetText() const {
  return base::WideToUTF8(text_->text);
}

}  // namespace nu
