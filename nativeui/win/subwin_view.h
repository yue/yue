// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SUBWIN_VIEW_H_
#define NATIVEUI_WIN_SUBWIN_VIEW_H_

#include "nativeui/win/window_impl.h"

namespace nu {

// The base class for implementing sub-window based UI.
class SubwinView : public WindowImpl {
 public:
  SubwinView(base::StringPiece16 class_name = L"",
             DWORD window_style = kWindowDefaultChildStyle,
             DWORD window_ex_style = 0);
  ~SubwinView() override;

 protected:
  CR_BEGIN_MSG_MAP_EX(SubwinView, WindowImpl)
    CR_MSG_WM_SIZE(OnSize)
  CR_END_MSG_MAP()

  virtual void OnSize(UINT param, const gfx::Size& size);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SUBWIN_VIEW_H_
