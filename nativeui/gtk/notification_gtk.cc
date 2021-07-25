// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/gtk/notification_gtk.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "nativeui/app.h"
#include "nativeui/notification_center.h"

namespace nu {

namespace {

void AddActionToNotification(GVariantBuilder* actions_builder,
                             base::StringPiece action_id,
                             base::StringPiece button_label) {
  g_variant_builder_add(actions_builder, "s", action_id.data());
  g_variant_builder_add(actions_builder, "s", button_label.data());
}

// Callback used by GLib when the "Notify" message completes for the
// first time.
void NotifyCompleteReceiver(GObject* source_object,
                            GAsyncResult* result,
                            Notification* notification) {
  GDBusProxy* proxy = G_DBUS_PROXY(source_object);
  GVariant* value = g_dbus_proxy_call_finish(proxy, result, nullptr);
  if (!value) {
    // The message might have been cancelled, in which case
    // |notification| points to a destroyed NotificationData.
    return;
  }

  NotificationData* data = notification->GetNative();
  data->cancellable.reset();
  if (value && g_variant_is_of_type(value, G_VARIANT_TYPE("(u)")))
    g_variant_get(value, "(u)", &data->dbus_id);

  if (data->dbus_id) {
    if (data->should_close) {
      data->should_close = false;
      notification->Close();
      return;
    }
    if (data->should_update) {
      data->should_update = false;
      notification->Show();
      return;
    }
  }

  NotificationCenter::GetCurrent()->on_notification_show.Emit(
      notification->GetInfo());
}

}  // namespace

const char kNotificationActionPrefix[] = "action-";

NotificationData::NotificationData() = default;

NotificationData::~NotificationData() = default;

void Notification::Show() {
  GDBusProxy* proxy = NotificationCenter::GetCurrent()->GetNative();
  if (!proxy)
    return;

  // Previous |Show| is still pending.
  if (notification_->cancellable) {
    notification_->should_update = true;
    return;
  }
  notification_->cancellable.reset(g_cancellable_new());

  GVariantBuilder actions_builder;
  g_variant_builder_init(&actions_builder, G_VARIANT_TYPE("as"));
  for (const auto& a : notification_->actions)
    AddActionToNotification(&actions_builder,
                            kNotificationActionPrefix + a.info, a.title);
  // Special case: the pair ("default", "") will not add a button,
  // but instead makes the entire notification clickable.
  AddActionToNotification(&actions_builder, "default", "");

  GVariantBuilder hints_builder;
  g_variant_builder_init(&hints_builder, G_VARIANT_TYPE("a{sv}"));
  if (notification_->silent) {
    g_variant_builder_add(&hints_builder, "{sv}", "suppress-sound",
                          g_variant_new_boolean(true));
  }
  if (!notification_->image_path.empty()) {
    g_variant_builder_add(
        &hints_builder, "{sv}", "image-path",
        g_variant_new_string(notification_->image_path.c_str()));
  }
  std::string desktop_name = App::GetCurrent()->GetID();
  if (!desktop_name.empty()) {
    g_variant_builder_add(&hints_builder, "{sv}", "desktop-entry",
                          g_variant_new_string(desktop_name.c_str()));
  }

  GVariant* parameters = g_variant_new(
      "(susssasa{sv}i)", App::GetCurrent()->GetName().c_str(),
      notification_->dbus_id, "" /* app_icon */,
      notification_->title.c_str(), notification_->body.c_str(),
      &actions_builder, &hints_builder, -1);
  g_dbus_proxy_call(
      proxy, "Notify", parameters, G_DBUS_CALL_FLAGS_NONE, -1,
      notification_->cancellable,
      reinterpret_cast<GAsyncReadyCallback>(&NotifyCompleteReceiver), this);

  NotificationCenter::GetCurrent()->AddNotification(this);
}

void Notification::Close() {
  if (!notification_->dbus_id) {
    notification_->should_close = true;
    return;
  }
  GDBusProxy* proxy = NotificationCenter::GetCurrent()->GetNative();
  if (!proxy)
    return;
  g_dbus_proxy_call(proxy, "CloseNotification",
                    g_variant_new("(u)", notification_->dbus_id),
                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
  notification_->dbus_id = 0;
}

void Notification::SetTitle(const std::string& title) {
  notification_->title = title;
}

void Notification::SetBody(const std::string& body) {
  notification_->body = body;
}

void Notification::SetInfo(const std::string& info) {
  notification_->info = info;
}

std::string Notification::GetInfo() const {
  return notification_->info;
}

void Notification::SetSilent(bool silent) {
  notification_->silent = silent;
}

void Notification::SetImagePath(const base::FilePath& path) {
  notification_->image_path = path.value();
}

void Notification::SetActions(const std::vector<Action>& actions) {
  notification_->actions = actions;
}

void Notification::PlatformInit() {
  notification_ = new NotificationData;
}

void Notification::PlatformDestroy() {
  if (notification_->cancellable)
    g_cancellable_cancel(notification_->cancellable);
  delete notification_;
}

}  // namespace nu
