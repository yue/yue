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

Label::Label(const std::string& text)
    : text_(new AttributedText(text, TextFormat({TextAlign::Center,
                                                 TextAlign::Center,
                                                 true, false}))),
      use_system_color_(true),
      system_color_(Color::Get(Color::Name::Text)) {
  Init();
}

Label::Label(scoped_refptr<AttributedText> text)
    : text_(std::move(text)),
      use_system_color_(false) {
  Init();
}

Label::~Label() {}

void Label::SetText(const std::string& text) {
  text_->SetText(text);
  MarkDirty();
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
  use_system_color_ = false;
  text_ = std::move(text);
  MarkDirty();
}

void Label::UpdateColor() {
  if (!use_system_color_)
    return;
  Color color = Color::Get(Color::Name::Text);
  if (color == system_color_)
    return;
  text_->SetColor(color);
  system_color_ = color;
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

void Label::SetFont(scoped_refptr<Font> font) {
  text_->SetFont(font);
  View::SetFont(std::move(font));
  MarkDirty();  // layout has changed
}

void Label::SetColor(Color color) {
  use_system_color_ = false;
  text_->SetColor(color);
  View::SetColor(color);
}

}  // namespace nu
