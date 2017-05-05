// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/subwin_view.h"

#include "base/win/scoped_hdc.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/font.h"
#include "nativeui/state.h"
#include "nativeui/win/scroll_win.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

SubwinView::SubwinView(View* delegate,
                       base::StringPiece16 class_name,
                       DWORD window_style, DWORD window_ex_style)
    : Win32Window(class_name, State::GetCurrent()->GetSubwinHolder(),
                  window_style, window_ex_style),
      ViewImpl(ControlType::Subwin, delegate),
      proc_(SetWindowProc(hwnd(), &WndProc)) {
  // Create HFONT from default system font.
  base::win::ScopedCreateDC mem_dc(CreateCompatibleDC(NULL));
  Gdiplus::Graphics context(mem_dc.Get());
  LOGFONTW logfont;
  State::GetCurrent()->GetDefaultFont()->GetNative()->GetLogFontW(
      &context, &logfont);
  font_.reset(CreateFontIndirect(&logfont));
  // Use it as control's default font.
  SendMessage(hwnd(), WM_SETFONT, reinterpret_cast<WPARAM>(font_.get()), TRUE);
}

SubwinView::~SubwinView() {
}

void SubwinView::SizeAllocate(const Rect& size_allocation) {
  ViewImpl::SizeAllocate(size_allocation);

  // Manually hide the control if it is not visible, this is necessary because
  // the control may be inside a Scroll.
  Rect clipped = GetClippedRect();
  if (clipped.IsEmpty()) {
    ::ShowWindow(hwnd(), SW_HIDE);
    return;
  }

  // Implement clipping by setting window region.
  clipped.Offset(-size_allocation.x(), -size_allocation.y());
  HRGN region = ::CreateRectRgn(clipped.x(), clipped.y(),
                                clipped.right(), clipped.bottom());
  ::SetWindowRgn(hwnd(), region, FALSE);  // SetWindowRgn takes ownership

  ::ShowWindow(hwnd(), SW_SHOWNOACTIVATE);
  SetWindowPos(hwnd(), NULL,
               size_allocation.x(), size_allocation.y(),
               size_allocation.width(), size_allocation.height(),
               SWP_NOACTIVATE | SWP_NOZORDER);
  RedrawWindow(hwnd(), NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void SubwinView::SetParent(ViewImpl* parent) {
  ViewImpl::SetParent(parent);
  ::SetParent(hwnd(),
              parent && parent->window()
                  ? parent->window()->hwnd()
                  : State::GetCurrent()->GetSubwinHolder());
}

void SubwinView::BecomeContentView(WindowImpl* parent) {
  ViewImpl::BecomeContentView(parent);
  ::SetParent(hwnd(), parent ? parent->hwnd()
                             : State::GetCurrent()->GetSubwinHolder());
}

void SubwinView::Invalidate(const Rect& dirty) {
  InvalidateRect(hwnd(), NULL, TRUE);
}

void SubwinView::SetFocus(bool focus) {
  ::SetFocus(focus ? hwnd() : ::GetParent(hwnd()));
  if (!focus)
    ::SendMessage(hwnd(), WM_KILLFOCUS, NULL, NULL);
}

bool SubwinView::HasFocus() const {
  return ::GetFocus() == hwnd();
}

void SubwinView::SetVisible(bool visible) {
  ViewImpl::SetVisible(visible);
  ::ShowWindow(hwnd(), visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void SubwinView::SetBackgroundColor(Color color) {
  bg_brush_.reset(CreateSolidBrush(color.ToCOLORREF()));
  ViewImpl::SetBackgroundColor(color);
}

void SubwinView::Draw(PainterWin* painter, const Rect& dirty) {
  // There is nothing to draw in a sub window.
}

bool SubwinView::OnCtlColor(HDC dc, HBRUSH* brush) {
  if (!bg_brush_.get())
    return false;
  SetBkMode(dc, TRANSPARENT);
  *brush = bg_brush_.get();
  return true;
}

void SubwinView::OnChar(UINT ch, UINT repeat, UINT flags) {
  if (ch == VK_TAB && window())  // Switching focus.
    window()->AdvanceFocus();
  else
    SetMsgHandled(false);
}

void SubwinView::OnSetFocus(HWND hwnd) {
  // Notify the window that focus has changed.
  if (window())
    window()->focus_manager()->TakeFocus(delegate());
  SetMsgHandled(false);
}

LRESULT SubwinView::OnMouseClick(UINT message, WPARAM w_param, LPARAM l_param) {
  Win32Message msg = {message, w_param, l_param};
  if (!ViewImpl::OnMouseClick(&msg))
    SetMsgHandled(false);
  return 0;
}

LRESULT SubwinView::OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param) {
  Win32Message msg = {message, w_param, l_param};
  if (!ViewImpl::OnKeyEvent(&msg))
    SetMsgHandled(false);
  return 0;
}

// static
LRESULT SubwinView::WndProc(HWND hwnd,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param) {
  auto* self = reinterpret_cast<SubwinView*>(GetWindowUserData(hwnd));
  if (!self)  // could happen during destruction
    return 0;
  LRESULT lresult = 0;
  if (self->ProcessWindowMessage(hwnd, message, w_param, l_param, &lresult))
    return lresult;
  return CallWindowProc(self->proc_, hwnd, message, w_param, l_param);
}

}  // namespace nu
