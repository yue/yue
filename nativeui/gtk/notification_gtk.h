// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_GTK_NOTIFICATION_GTK_H_
#define NATIVEUI_GTK_NOTIFICATION_GTK_H_

#include "nativeui/notification.h"

#include <gio/gio.h>

#include <string>
#include <vector>

#include "nativeui/gtk/scoped_gobject.h"

namespace nu {

extern const char kNotificationActionPrefix[];

struct NotificationData {
  std::string title;
  std::string body;
  std::string info;
  GVariantBuilder* actions_builder;
  GVariantBuilder* hints_builder;

  // The ID used by the notification server. Will be 0 until the
  // first "Notify" message completes.
  uint32_t dbus_id = 0;

  // Used to cancel the initial "Notify" message.
  ScopedGObject<GCancellable> cancellable;

  // If true, indicates the notification should be closed once
  // |dbus_id| becomes available.
  bool should_close = false;

  NotificationData();
  ~NotificationData();
};

}  // namespace nu

#endif  // NATIVEUI_GTK_NOTIFICATION_GTK_H_
