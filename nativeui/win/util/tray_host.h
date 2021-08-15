// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_UTIL_TRAY_HOST_H_
#define NATIVEUI_WIN_UTIL_TRAY_HOST_H_

#include <vector>

#include "nativeui/win/util/win32_window.h"

namespace nu {

class TrayImpl;

class TrayHost : public Win32Window {
 public:
  static const UINT kMessage = WM_APP + 1;

  TrayHost();
  ~TrayHost() final;

  UINT NextIconId();

  void Add(TrayImpl* tray);
  void Remove(TrayImpl* tray);

 protected:
  bool ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param,
                            LRESULT* result) override;

 private:
  std::vector<TrayImpl*> tray_icons_;

  // The unique icon ID we will assign to the next icon.
  UINT next_icon_id_ = 0;

  // The message ID of the "TaskbarCreated" message, sent to us when we need to
  // reset our status icons.
  UINT taskbar_created_message_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_TRAY_HOST_H_
