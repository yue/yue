// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_TOOLTIP_HOST_H_
#define NATIVEUI_WIN_UTIL_TOOLTIP_HOST_H_

#include "nativeui/win/util/win32_window.h"

namespace nu {

class TooltipHost : public Win32Window {
 public:
  TooltipHost();
  ~TooltipHost() override;

  int GetNextId() { return ++next_tooltip_id_; }
  void AddTooltip(HWND win, UINT_PTR id, LPWSTR str, Rect rect);
  void RemoveTooltip(HWND win, UINT_PTR id);
  void UpdateTooltipRect(HWND win, UINT_PTR id, Rect rect);
  void SetEnabled(bool enable);

 private:
  int next_tooltip_id_ = 0;
  bool enabled_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_TOOLTIP_HOST_H_
