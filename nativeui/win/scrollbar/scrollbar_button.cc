// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/scrollbar_button.h"

#include "nativeui/win/scrollbar/scrollbar.h"

namespace nu {

ScrollbarButton::ScrollbarButton(Type type, Scrollbar* scrollbar)
    : ViewImpl(ControlType::ScrollbarButton, nullptr),
      repeater_(base::Bind(&ScrollbarButton::OnClick, base::Unretained(this))),
      type_(type),
      scrollbar_(scrollbar) {
}

ScrollbarButton::~ScrollbarButton() {
}

void ScrollbarButton::OnMouseEnter() {
  set_state(ControlState::Hovered);
  Invalidate();
}

void ScrollbarButton::OnMouseLeave() {
  set_state(ControlState::Normal);
  repeater_.Stop();
  Invalidate();
}

bool ScrollbarButton::OnMouseClick(UINT message, UINT flags, const Point&) {
  if (message == WM_LBUTTONDOWN) {
    set_state(ControlState::Pressed);
    OnClick();
    repeater_.Start();
  } else {
    set_state(ControlState::Hovered);
    repeater_.Stop();
  }
  Invalidate();
  return true;
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

void ScrollbarButton::OnClick() {
  if (type_ == Up || type_ == Left)
    scrollbar_->LineUp();
  else
    scrollbar_->LineDown();
}

}  // namespace nu
