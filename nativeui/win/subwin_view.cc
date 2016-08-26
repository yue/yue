// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/subwin_view.h"

#include "nativeui/win/subwin_holder.h"

namespace nu {

SubwinView::SubwinView(base::StringPiece16 class_name,
                       DWORD window_style, DWORD window_ex_style)
    : WindowImpl(class_name, SubwinHolder::GetInstance()->hwnd(),
                 window_style, window_ex_style),
      BaseView(false) {
}

SubwinView::~SubwinView() {
}

void SubwinView::SetPixelBounds(const Rect& bounds) {
  BaseView::SetPixelBounds(bounds);

  // Calculate the bounds relative to parent HWND.
  Point pos(bounds.origin());
  if (parent()) {
    Point offset = parent()->GetWindowPixelOrigin();
    pos.set_x(pos.x() + offset.x());
    pos.set_y(pos.y() + offset.y());
  }

  SetWindowPos(hwnd(), NULL, pos.x(), pos.y(), bounds.width(), bounds.height(),
               SWP_NOACTIVATE | SWP_NOZORDER);
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void SubwinView::SetParent(BaseView* parent) {
  BaseView::SetParent(parent);
  ::SetParent(hwnd(),
              parent && parent->window() ? parent->window()->hwnd()
                                         : SubwinHolder::GetInstance()->hwnd());
}

void SubwinView::BecomeContentView(WindowImpl* parent) {
  BaseView::BecomeContentView(parent);
  ::SetParent(hwnd(), parent ? parent->hwnd()
                             : SubwinHolder::GetInstance()->hwnd());
}

}  // namespace nu
