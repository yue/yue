// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/util/tray_host.h"

#include <commctrl.h>

#include <algorithm>

#include "nativeui/win/tray_win.h"

namespace nu {

namespace {

// |kBaseIconId| is 2 to avoid conflicts with plugins that hard-code id 1.
const UINT kBaseIconId = 2;

}  // namespace

TrayHost::TrayHost() {
  // If the taskbar is re-created after we start up, we have to rebuild all of
  // our icons.
  taskbar_created_message_ = RegisterWindowMessage(TEXT("TaskbarCreated"));
}

TrayHost::~TrayHost() {
}

UINT TrayHost::NextIconId() {
  return kBaseIconId + next_icon_id_++;
}

void TrayHost::Add(TrayImpl* tray) {
  tray_icons_.push_back(tray);
}

void TrayHost::Remove(TrayImpl* tray) {
  tray_icons_.erase(std::remove(tray_icons_.begin(), tray_icons_.end(), tray),
                    tray_icons_.end());
}

bool TrayHost::ProcessWindowMessage(
    HWND, UINT message, WPARAM w_param, LPARAM l_param, LRESULT* result) {
  *result = TRUE;
  if (message == taskbar_created_message_) {
    // We need to reset all of our icons because the taskbar went away.
    for (auto* tray : tray_icons_)
      tray->ResetIcon();
    return true;
  } else if (message == kMessage) {
    // Find the selected status icon.
    TrayImpl* tray = nullptr;
    for (auto* t : tray_icons_) {
      if (t->icon_id() == w_param) {
        tray = t;
        break;
      }
    }

    // It is possible for this procedure to be called with an obsolete icon
    // id.  In that case we should just return early before handling any
    // actions.
    if (!tray)
      return true;

    switch (l_param) {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_CONTEXTMENU:
        tray->HandleClickEvent(l_param);
        return true;
    }
  }
  return false;
}

}  // namespace nu
