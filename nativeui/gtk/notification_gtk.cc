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

  if (data->dbus_id && data->should_close) {
    notification->Close();
    return;
  }

  NotificationCenter::GetCurrent()->on_notification_show.Emit(
      notification->GetInfo());
}

}  // namespace

const char kNotificationActionPrefix[] = "action-";

NotificationData::NotificationData()
    : actions_builder(g_variant_builder_new(G_VARIANT_TYPE("as"))),
      hints_builder(g_variant_builder_new(G_VARIANT_TYPE("a{sv}"))) {
}

NotificationData::~NotificationData() {
  g_variant_builder_unref(actions_builder);
  g_variant_builder_unref(hints_builder);
}

void Notification::Show() {
  GDBusProxy* proxy = NotificationCenter::GetCurrent()->GetNative();
  if (!proxy)
    return;

  if (notification_->cancellable)  // previous |Show| is still pending.
    return;
  notification_->cancellable.reset(g_cancellable_new());

  GVariant* parameters = g_variant_new(
      "(susssasa{sv}i)", App::GetCurrent()->GetName().c_str(),
      notification_->dbus_id, "" /* app_icon */,
      notification_->title.c_str(), notification_->body.c_str(),
      notification_->actions_builder, notification_->hints_builder, -1);
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
  g_variant_builder_add(notification_->hints_builder, "{sv}",
                        "suppress-sound", g_variant_new_boolean(silent));
}

void Notification::SetImagePath(const base::FilePath& path) {
  g_variant_builder_add(
      notification_->hints_builder, "{sv}", "image-path",
      g_variant_new_string(path.value().c_str()));
}

void Notification::SetActions(const std::vector<Action>& actions) {
  for (const auto& a : actions)
    AddActionToNotification(notification_->actions_builder,
                            kNotificationActionPrefix + a.info, a.title);
  // Special case: the pair ("default", "") will not add a button,
  // but instead makes the entire notification clickable.
  AddActionToNotification(notification_->actions_builder, "default", "");
}

void Notification::PlatformInit() {
  notification_ = new NotificationData;

  std::string desktop_name = App::GetCurrent()->GetID();
  if (!desktop_name.empty()) {
    g_variant_builder_add(notification_->hints_builder, "{sv}", "desktop-entry",
                          g_variant_new_string(desktop_name.c_str()));
  }
}

void Notification::PlatformDestroy() {
  if (notification_->cancellable)
    g_cancellable_cancel(notification_->cancellable);
  delete notification_;
}

}  // namespace nu
