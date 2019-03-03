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
  // Apply default font.
  SetFont(font());
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
  if (clipped.x() > 0 ||
      clipped.y() > 0 ||
      clipped.width() < size_allocation.width() ||
      clipped.height() < size_allocation.height()) {
    HRGN region = ::CreateRectRgn(clipped.x(), clipped.y(),
                                  clipped.right(), clipped.bottom());
    ::SetWindowRgn(hwnd(), region, FALSE);  // SetWindowRgn takes ownership
  } else {
    SetWindowRgn(hwnd(), NULL, FALSE);
  }

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
  if (transprent_background_)
    UpdateTransparentBackgroundBrush();
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
  if (focus) {
    ::SetFocus(hwnd());
  } else {
    if (window())
      window()->focus_manager()->RemoveFocus(this);
    ::SendMessage(hwnd(), WM_KILLFOCUS, 0, 0L);
  }
}

bool SubwinView::HasFocus() const {
  return ::GetFocus() == hwnd();
}

void SubwinView::VisibilityChanged() {
  ViewImpl::VisibilityChanged();
  ::ShowWindow(hwnd(), is_tree_visible() ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void SubwinView::SetFont(Font* new_font) {
  ViewImpl::SetFont(new_font);
  // Use it as control's default font.
  SendMessage(hwnd(), WM_SETFONT,
              reinterpret_cast<WPARAM>(new_font->GetHFONT(hwnd())), TRUE);
}

void SubwinView::SetBackgroundColor(Color color) {
  ViewImpl::SetBackgroundColor(color);
  if (color.transparent())
    SetTransparentBackground();
  else
    bg_brush_.reset(CreateSolidBrush(color.ToCOLORREF()));
}

void SubwinView::Draw(PainterWin* painter, const Rect& dirty) {
  // There is nothing to draw in a sub window.
}

void SubwinView::SetTransparentBackground() {
  transprent_background_ = true;
  UpdateTransparentBackgroundBrush();
}

bool SubwinView::OnCtlColor(HDC dc, HBRUSH* brush) {
  ::SetTextColor(dc, color().ToCOLORREF());
  if (!bg_brush_.get())
    return false;
  ::SetBkMode(dc, TRANSPARENT);
  *brush = bg_brush_.get();
  return true;
}

void SubwinView::OnChar(UINT ch, UINT repeat, UINT flags) {
  if (switch_focus_on_tab_ && ch == VK_TAB && window())  // switching focus
    window()->AdvanceFocus();
  else
    SetMsgHandled(false);
}

BOOL SubwinView::OnSetCursor(HWND hwnd, UINT hittest, UINT message) {
  if (cursor()) {
    ::SetCursor(cursor()->GetNative());
    return TRUE;
  }
  SetMsgHandled(false);
  return FALSE;
}

void SubwinView::OnSetFocus(HWND hwnd) {
  // Notify the window that focus has changed.
  if (window())
    window()->focus_manager()->TakeFocus(this);
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

LRESULT SubwinView::OnMouseWheelFromSelf(
    UINT message, WPARAM w_param, LPARAM l_param) {
  if (window()) {
    // Pass the event to window if not happened inside the control.
    POINT p = { CR_GET_X_LPARAM(l_param), CR_GET_Y_LPARAM(l_param) };
    ::ScreenToClient(window()->hwnd(), &p);
    if (!size_allocation().Contains(Point(p))) {
      ::SendMessage(window()->hwnd(), message, w_param, l_param);
      return 0;
    }
  }
  SetMsgHandled(false);
  return 0;
}

void SubwinView::UpdateTransparentBackgroundBrush() {
  if (!window() || !parent()) {
    bg_brush_.reset();
    return;
  }

  // Search for the opaque parent.
  ViewImpl* p = parent();
  while (p && p->background_color().transparent())
    p = p->parent();
  Color b = p ? p->background_color() : window()->background_color();
  bg_brush_.reset(CreateSolidBrush(b.ToCOLORREF()));
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
