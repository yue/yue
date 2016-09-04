// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/subwin_view.h"

#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/font.h"
#include "nativeui/state.h"

namespace nu {

SubwinView::SubwinView(base::StringPiece16 class_name,
                       DWORD window_style, DWORD window_ex_style)
    : WindowImpl(class_name, State::current()->GetSubwinHolder(),
                 window_style, window_ex_style),
      BaseView(false) {
  // Create HFONT from default system font.
  base::win::ScopedCreateDC mem_dc(CreateCompatibleDC(NULL));
  Gdiplus::Graphics context(mem_dc.Get());
  LOGFONTW logfont;
  Font().GetNativeFont()->GetLogFontW(&context, &logfont);
  font_.reset(CreateFontIndirect(&logfont));
  // Use it as control's default font.
  SendMessage(hwnd(), WM_SETFONT, reinterpret_cast<WPARAM>(font_.get()), TRUE);
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
                                         : State::current()->GetSubwinHolder());
}

void SubwinView::BecomeContentView(WindowImpl* parent) {
  BaseView::BecomeContentView(parent);
  ::SetParent(hwnd(), parent ? parent->hwnd()
                             : State::current()->GetSubwinHolder());
}

}  // namespace nu
