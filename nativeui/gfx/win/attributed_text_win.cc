// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/attributed_text.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/gfx/text.h"
#include "nativeui/gfx/win/direct_write.h"
#include "nativeui/gfx/win/drawing_effect.h"

namespace nu {

namespace {

inline int IndexToLength(const base::string16 str, int start, int end) {
  return end > -1 ? end - start : static_cast<int>(str.size()) - start;
}

inline DWRITE_FONT_STYLE ToDWriteType(Font::Style style) {
  switch (style) {
    case Font::Style::Italic:
      return DWRITE_FONT_STYLE_ITALIC;
    case Font::Style::Normal:
    default:
      return DWRITE_FONT_STYLE_NORMAL;
  }
}

inline DWRITE_FONT_WEIGHT ToDWriteType(Font::Weight weight) {
  switch (weight) {
    case Font::Weight::Thin:
      return DWRITE_FONT_WEIGHT_THIN;
    case Font::Weight::ExtraLight:
      return DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
    case Font::Weight::Light:
      return DWRITE_FONT_WEIGHT_LIGHT;
    case Font::Weight::Medium:
      return DWRITE_FONT_WEIGHT_MEDIUM;
    case Font::Weight::SemiBold:
      return DWRITE_FONT_WEIGHT_SEMI_BOLD;
    case Font::Weight::Bold:
      return DWRITE_FONT_WEIGHT_BOLD;
    case Font::Weight::ExtraBold:
      return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
    case Font::Weight::Black:
      return DWRITE_FONT_WEIGHT_BLACK;
    case Font::Weight::Normal:
    default:
      return DWRITE_FONT_WEIGHT_NORMAL;
  }
}

}  // namespace

AttributedText::AttributedText(const std::string& text,
                               const TextFormat& format)
    : format_(format), original_text_(base::UTF8ToUTF16(text)) {
  CHECK(CreateTextLayout(original_text_, format, &text_));
}

AttributedText::~AttributedText() {
  text_->Release();
}

void AttributedText::PlatformSetFontFor(Font* font, int start, int end) {
  DWRITE_TEXT_RANGE range = {start, IndexToLength(original_text_, start, end)};
  text_->SetFontFamilyName(font->GetName16().c_str(), range);
  text_->SetFontSize(font->GetSize(), range);
  text_->SetFontStyle(ToDWriteType(font->GetStyle()), range);
  text_->SetFontWeight(ToDWriteType(font->GetWeight()), range);
}

void AttributedText::PlatformSetColorFor(Color color, int start, int end) {
  DWRITE_TEXT_RANGE range = {start, IndexToLength(original_text_, start, end)};
  scoped_refptr<DrawingEffect> drawing_effect(new DrawingEffect);
  drawing_effect->fg_color = color;
  text_->SetDrawingEffect(drawing_effect.get(), range);
}

RectF AttributedText::GetBoundsFor(const SizeF& size) const {
  text_->SetMaxWidth(size.width());
  text_->SetMaxHeight(size.height());
  DWRITE_TEXT_METRICS metrics = {0};
  text_->GetMetrics(&metrics);
  return RectF(metrics.left, metrics.top, metrics.width, metrics.height);
}

std::string AttributedText::GetText() const {
  return base::UTF16ToUTF8(original_text_);
}

}  // namespace nu
