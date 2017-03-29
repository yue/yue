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
  SubwinView(base::StringPiece16 class_name = L"",
             DWORD window_style = kWindowDefaultChildStyle,
             DWORD window_ex_style = 0);
  ~SubwinView() override;

  void SizeAllocate(const Rect& size_allocation) override;
  void SetParent(ViewImpl* parent) override;
  void BecomeContentView(WindowImpl* parent) override;
  void Invalidate(const Rect& dirty) override;
  void SetFocus(bool focus) override;
  bool IsFocused() const override;
  void SetVisible(bool visible) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

  // Rerouted from parent window
  virtual void OnCommand(UINT code, int command) {}
  virtual bool OnCtlColor(HDC dc, HBRUSH* brush);

 private:
  base::win::ScopedHFONT font_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SUBWIN_VIEW_H_
