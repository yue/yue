// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/notifications/notification_activator.h"

#include <string>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/notreached.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/notification_center.h"
#include "nativeui/win/notifications/notification_builder.h"

namespace nu {

NotificationActivator::NotificationActivator() = default;

NotificationActivator::~NotificationActivator() = default;

HRESULT NotificationActivator::Activate(
    LPCWSTR app_user_model_id,
    LPCWSTR invoked_args,
    const NOTIFICATION_USER_INPUT_DATA* data,
    ULONG count) {
  if (!invoked_args) {
    NOTREACHED() << "null invoked_args";
    return S_OK;
  }

  auto* center = NotificationCenter::GetCurrent();
  if (!center->on_toast_activate.IsEmpty()) {
    std::vector<NotificationCenter::InputData> inputs;
    inputs.reserve(count);
    for (ULONG i = 0; i < count; ++i)
      inputs.push_back({data[i].Key, data[i].Value});
    if (center->on_toast_activate.Emit(app_user_model_id, invoked_args,
                                       std::move(inputs)))
      return S_OK;
  }

  if (base::StartsWith(invoked_args, kNotificationTypeClick)) {
    center->on_notification_click.Emit(
        base::WideToUTF8(invoked_args).substr(wcslen(kNotificationTypeClick)));
  } else if (base::StartsWith(invoked_args, kNotificationTypeAction)) {
    center->on_notification_action.Emit(
        base::WideToUTF8(invoked_args).substr(wcslen(kNotificationTypeAction)));
  } else if (base::StartsWith(invoked_args, kNotificationTypeReply)) {
    std::string reply;
    for (ULONG i = 0; i < count; ++i) {
      if (lstrcmpW(kReplyInputId, data[i].Key) == 0) {
        reply = base::WideToUTF8(data[i].Value);
        break;
      }
    }
    center->on_notification_reply.Emit(
        base::WideToUTF8(invoked_args).substr(wcslen(kNotificationTypeReply)),
        std::move(reply));
  }
  return S_OK;
}

}  // namespace nu
