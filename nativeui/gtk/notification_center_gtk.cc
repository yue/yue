// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/notification_center.h"

#include <gio/gio.h>

#include "base/containers/contains.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "nativeui/gtk/notification_gtk.h"

namespace nu {

namespace {

const char kFreedesktopNotificationsName[] = "org.freedesktop.Notifications";
const char kFreedesktopNotificationsPath[] = "/org/freedesktop/Notifications";

void NotificationGSignalReceiver(GDBusProxy* proxy,
                                 const char* sender_name,
                                 const char* sender_signal,
                                 GVariant* parameters,
                                 NotificationCenter* center) {
  // Records the notification has responded to actions.
  static std::set<uint32_t> responded;

  uint32_t dbus_id = 0;
  if (strcmp("NotificationClosed", sender_signal) == 0 &&
      g_variant_is_of_type(parameters, G_VARIANT_TYPE("(uu)"))) {
    uint32_t reason;
    g_variant_get(parameters, "(uu)", &dbus_id, &reason);
    // Emit close event if it is not closed due to responding to actions.
    auto* notification = center->FindNotification(dbus_id);
    if (base::Contains(responded, dbus_id))
      responded.erase(dbus_id);
    else
      center->on_notification_close.Emit(
          notification ? notification->GetNative()->info : std::string());
    if (notification)
      center->RemoveNotification(notification);
  } else if (strcmp("ActionInvoked", sender_signal) == 0 &&
             g_variant_is_of_type(parameters, G_VARIANT_TYPE("(us)"))) {
    const gchar* action = nullptr;
    g_variant_get(parameters, "(u&s)", &dbus_id, &action);
    DCHECK(action);
    responded.insert(dbus_id);
    // The "default" is a special action for clicking the notification body.
    if (strcmp(action, "default") == 0) {
      auto* notification = center->FindNotification(dbus_id);
      center->on_notification_click.Emit(
          notification ? notification->GetNative()->info : std::string());
    } else if (base::StartsWith(action, kNotificationActionPrefix)) {
      center->on_notification_action.Emit(
          action + strlen(kNotificationActionPrefix));
    }
  }
}

}  // namespace

void NotificationCenter::Clear() {
  auto copy = notifications_;  // might be modified while iterating
  for (auto& n : copy)
    n->Close();
}

void NotificationCenter::AddNotification(Notification* notification) {
  notifications_.insert(notification);
}

void NotificationCenter::RemoveNotification(Notification* notification) {
  notifications_.erase(notification);
}

Notification* NotificationCenter::FindNotification(uint32_t dbus_id) const {
  for (auto& n : notifications_) {
    if (n->GetNative()->dbus_id == dbus_id)
      return n.get();
  }
  return nullptr;
}

void NotificationCenter::PlatformInit() {
  center_ = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION,
      static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                                   G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START),
      nullptr, kFreedesktopNotificationsName, kFreedesktopNotificationsPath,
      kFreedesktopNotificationsName, nullptr, nullptr);
  if (!center_) {
    LOG(ERROR) << "Unable to connect to DBUS session: "
               << kFreedesktopNotificationsName;
    return;
  }
  proxy_signal_handler_ = g_signal_connect(
      center_, "g-signal", G_CALLBACK(NotificationGSignalReceiver), this);
}

void NotificationCenter::PlatformDestroy() {
  if (proxy_signal_handler_)
    g_signal_handler_disconnect(center_, proxy_signal_handler_);
  g_object_unref(center_);
}

}  // namespace nu
