// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/scrollbar_button.h"

#include "nativeui/events/win/event_win.h"
#include "nativeui/win/scrollbar/scrollbar.h"

namespace nu {

ScrollbarButton::ScrollbarButton(Type type, Scrollbar* scrollbar)
    : Clickable(ControlType::View, nullptr),
      repeater_([this] { this->OnClick(); }),
      type_(type),
      scrollbar_(scrollbar) {
}

ScrollbarButton::~ScrollbarButton() {
}

void ScrollbarButton::OnClick() {
  if (type_ == Up || type_ == Left)
    scrollbar_->LineUp();
  else
    scrollbar_->LineDown();
}

void ScrollbarButton::OnMouseLeave(NativeEvent event) {
  repeater_.Stop();
  Clickable::OnMouseLeave(event);
}

bool ScrollbarButton::OnMouseClick(NativeEvent event) {
  if (event->message == WM_LBUTTONDOWN)
    repeater_.Start();
  else
    repeater_.Stop();
  return Clickable::OnMouseClick(event);
}

void ScrollbarButton::Draw(PainterWin* painter, const Rect& dirty) {
  NativeTheme::Part part = NativeTheme::Part::ScrollbarUpArrow;
  switch (type_) {
    case Up:
      part = NativeTheme::Part::ScrollbarUpArrow;
      break;
    case Down:
      part = NativeTheme::Part::ScrollbarDownArrow;
      break;
    case Left:
      part = NativeTheme::Part::ScrollbarLeftArrow;
      break;
    case Right:
      part = NativeTheme::Part::ScrollbarRightArrow;
      break;
  }
  NativeTheme::ExtraParams params;
  params.scrollbar_arrow = params_;
  painter->DrawNativeTheme(
      part, state(), Rect(size_allocation().size()), params);
}

}  // namespace nu
