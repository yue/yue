// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/font.h"
#include "nativeui/system.h"
#include "third_party/yoga/yoga/Yoga.h"

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

Label::Label() {
  TakeOverView(PlatformCreate());
  YGNodeSetMeasureFunc(node(), MeasureLabel);

  // Default color and font.
  font_ = System::GetDefaultFont();
  color_ = System::GetColor(System::Color::Text);
}

Label::Label(const std::string& text) : Label() {
  SetText(text);
}

Label::Label(AttributedText* text) : Label() {
  SetAttributedText(text);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  // Apply current font and color.
  auto* attributed_text = new AttributedText(
      text, {TextAlign::Center, TextAlign::Center, true, false});
  attributed_text->SetFont(font_.get());
  attributed_text->SetColor(color_);

  SetAttributedText(attributed_text);
  is_simple_text_ = true;  // must be changed after the SetAttributedText
}

std::string Label::GetText() const {
  return text_->GetText();
}

void Label::SetAttributedText(AttributedText* text) {
  is_simple_text_ = false;
  PlatformSetAttributedText(text);
  text_ = text;
  MarkDirty();
}

void Label::MarkDirty() {
  YGNodeMarkDirty(node());
  SchedulePaint();
}

const char* Label::GetClassName() const {
  return kClassName;
}

void Label::SetFont(Font* font) {
  font_ = font;
  if (is_simple_text_) {
    text_->SetFont(font);
    MarkDirty();  // layout has changed
  }
}

void Label::SetColor(Color color) {
  color_ = color;
  if (is_simple_text_) {
    text_->SetColor(color);
    SchedulePaint();  // only redraw
  }
}

}  // namespace nu
