// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_NOTIFICATIONS_NOTIFICATION_BUILDER_H_
#define NATIVEUI_WIN_NOTIFICATIONS_NOTIFICATION_BUILDER_H_

#include <string>

#include "nativeui/win/notifications/winrt_util.h"

namespace nu {

struct NotificationImpl;

extern const wchar_t kNotificationTypeClick[];
extern const wchar_t kNotificationTypeAction[];
extern const wchar_t kNotificationTypeReply[];
extern const wchar_t kReplyInputId[];

std::wstring GetNotificationXMLRepresentation(NotificationImpl* notification);

mswr::ComPtr<IToastNotification> BuildNotification(
    NotificationImpl* notification);

mswr::ComPtr<INotificationData> CreateNotificationData();
HRESULT NotificationDataInsert(INotificationData* data,
                               base::WStringPiece key,
                               base::WStringPiece value);

}  // namespace nu

#endif  // NATIVEUI_WIN_NOTIFICATIONS_NOTIFICATION_BUILDER_H_
