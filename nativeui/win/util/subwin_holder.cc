// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/util/subwin_holder.h"

#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

SubwinHolder::SubwinHolder() {
}

SubwinHolder::~SubwinHolder() {
}

void SubwinHolder::OnCommand(UINT code, int command, HWND window) {
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  if (control)
    control->OnCommand(code, command);
}

LRESULT SubwinHolder::OnNotify(int id, LPNMHDR pnmh) {
  HWND window = pnmh->hwndFrom;
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  if (!control)
    return 0;
  return control->OnNotify(id, pnmh);
}

HBRUSH SubwinHolder::OnCtlColorStatic(HDC dc, HWND window) {
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  if (!control)
    return NULL;
  HBRUSH brush = NULL;
  SetMsgHandled(control->OnCtlColor(dc, &brush));
  return brush;
}

void SubwinHolder::OnHScroll(UINT code, UINT pos, HWND window) {
  auto* control = reinterpret_cast<SubwinView*>(GetWindowUserData(window));
  if (control)
    control->OnHScroll(code, pos);
}

}  // namespace nu
