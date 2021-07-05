// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/notification_center.h"

#include "nativeui/state.h"

namespace nu {

// static
NotificationCenter* NotificationCenter::GetCurrent() {
  return State::GetCurrent()->GetNotificationCenter();
}

NotificationCenter::NotificationCenter() : weak_factory_(this) {
  on_notification_show.SetDelegate(this);
  on_notification_close.SetDelegate(this);
  on_notification_click.SetDelegate(this);
  on_notification_action.SetDelegate(this);
#if defined(OS_MAC)
  on_notification_reply.SetDelegate(this);
#endif
}

NotificationCenter::~NotificationCenter() {
  if (center_)
    PlatformDestroy();
}

void NotificationCenter::OnConnect(int) {
  if (!center_)
    PlatformInit();
}

}  // namespace nu
