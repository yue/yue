// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SUBWIN_VIEW_H_
#define NATIVEUI_WIN_SUBWIN_VIEW_H_

#include "base/win/scoped_gdi_object.h"
#include "nativeui/win/view_win.h"

namespace nu {

// The base class for implementing sub-window based UI.
class SubwinView : public Win32Window, public ViewImpl {
 public:
  SubwinView(View* delegate,
             base::StringPiece16 class_name = L"",
             DWORD window_style = kWindowDefaultChildStyle,
             DWORD window_ex_style = 0);
  ~SubwinView() override;

  void SizeAllocate(const Rect& size_allocation) override;
  void SetParent(ViewImpl* parent) override;
  void BecomeContentView(WindowImpl* parent) override;
  void Invalidate(const Rect& dirty) override;
  void SetFocus(bool focus) override;
  bool HasFocus() const override;
  void SetVisible(bool visible) override;
  void SetFont(Font* font) override;
  void SetBackgroundColor(Color color) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

  // Change behaviors.
  void set_switch_focus_on_tab(bool s) { switch_focus_on_tab_ = s; }

  // Rerouted from parent window
  virtual void OnCommand(UINT code, int command) {}
  virtual bool OnCtlColor(HDC dc, HBRUSH* brush);

 protected:
  CR_BEGIN_MSG_MAP_EX(SubwinView, Win32Window)
    CR_MSG_WM_CHAR(OnChar)
    CR_MSG_WM_SETFOCUS(OnSetFocus)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_LBUTTONDOWN, WM_MBUTTONDBLCLK, OnMouseClick)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_KEYDOWN, WM_KEYUP, OnKeyEvent)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_SYSKEYDOWN, WM_SYSKEYUP, OnKeyEvent)
  CR_END_MSG_MAP()

  void OnChar(UINT ch, UINT repeat, UINT flags);
  void OnSetFocus(HWND hwnd);
  LRESULT OnMouseClick(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param);

 private:
  // Subclass-ed window procedure.
  static LRESULT CALLBACK WndProc(HWND window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param);

  // Should switch focus when TAB is pressed.
  bool switch_focus_on_tab_ = true;

  base::win::ScopedHFONT font_;
  base::win::ScopedGDIObject<HBRUSH> bg_brush_;
  WNDPROC proc_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SUBWIN_VIEW_H_
