// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include <utility>

#include "nativeui/app.h"
#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/font.h"
#include "third_party/yoga/Yoga.h"

namespace nu {

namespace {

YGSize MeasureLabel(YGNodeRef node,
                    float width, YGMeasureMode mode,
                    float height, YGMeasureMode height_mode) {
  auto* label = static_cast<Label*>(YGNodeGetContext(node));
  SizeF size = label->GetAttributedText()
                    ->GetBoundsFor(SizeF(width, height)).size();
  size.Enlarge(1, 1);  // leave space for border
  return {std::ceil(size.width()), std::ceil(size.height())};
}

}  // namespace

// static
const char Label::kClassName[] = "Label";

Label::Label(const std::string& text) {
  Init();
  SetText(text);
}

Label::Label(scoped_refptr<AttributedText> text) {
  Init();
  SetAttributedText(std::move(text));
}

Label::~Label() {}

void Label::SetText(const std::string& text) {
  // Inherit settings from old text.
  const TextFormat format =
      text_ ? text_->GetFormat()
            : TextFormat({TextAlign::Center, TextAlign::Center, true, false});
  SetAttributedText(new AttributedText(text, format));
}

std::string Label::GetText() const {
  return text_->GetText();
}

void Label::SetAlign(TextAlign align) {
  TextFormat format = text_->GetFormat();
  format.align = align;
  text_->SetFormat(std::move(format));
  MarkDirty();
}

void Label::SetVAlign(TextAlign align) {
  TextFormat format = text_->GetFormat();
  format.valign = align;
  text_->SetFormat(std::move(format));
  MarkDirty();
}

void Label::SetAttributedText(scoped_refptr<AttributedText> text) {
  text_ = std::move(text);
  MarkDirty();
}

void Label::Init() {
  TakeOverView(PlatformCreate());
  YGNodeSetMeasureFunc(node(), MeasureLabel);
}

void Label::MarkDirty() {
  YGNodeMarkDirty(node());
  SchedulePaint();
}

const char* Label::GetClassName() const {
  return kClassName;
}

void Label::SetFont(Font* font) {
  text_->SetFont(font);
  View::SetFont(font);
  MarkDirty();  // layout has changed
}

void Label::SetColor(Color color) {
  text_->SetColor(color);
  View::SetColor(color);
}

}  // namespace nu
