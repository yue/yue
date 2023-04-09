// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/util/tooltip_host.h"

#include <commctrl.h>

namespace nu {

TooltipHost::TooltipHost()
    : Win32Window(TOOLTIPS_CLASS, NULL,
                  WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP) {}

TooltipHost::~TooltipHost() = default;

void TooltipHost::AddTooltip(HWND win, UINT_PTR id, LPWSTR str, Rect rect) {
  if (!str || str[0] == '\0')
    return;
  SetEnabled(true);
  TOOLINFO ti = {sizeof(ti)};
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = win,
  ti.uId = id;
  ti.lpszText = str;
  ti.rect = rect.ToRECT();
  ::SendMessage(hwnd(), TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
}

void TooltipHost::RemoveTooltip(HWND win, UINT_PTR id) {
  TOOLINFO ti = {sizeof(ti)};
  ti.hwnd = win,
  ti.uId = id;
  ::SendMessage(hwnd(), TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(&ti));
}

void TooltipHost::UpdateTooltipRect(HWND win, UINT_PTR id, Rect rect) {
  TOOLINFO ti = {sizeof(ti)};
  ti.hwnd = win,
  ti.uId = id;
  ti.rect = rect.ToRECT();
  ::SendMessage(hwnd(), TTM_NEWTOOLRECT, 0, reinterpret_cast<LPARAM>(&ti));
}

void TooltipHost::SetEnabled(bool enable) {
  if (enabled_ == enable)
    return;
  enabled_ = enable;
  ::SendMessage(hwnd(), TTM_ACTIVATE, enable ? TRUE : FALSE, 0);
}

}  // namespace nu
