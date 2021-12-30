// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_NOTIFICATIONS_NOTIFICATION_ACTIVATOR_H_
#define NATIVEUI_WIN_NOTIFICATIONS_NOTIFICATION_ACTIVATOR_H_

#include <NotificationActivationCallback.h>
#include <wrl/implements.h>

namespace nu {

// This class is used to create a COM component that exposes the
// INotificationActivationCallback interface, which is required for a Win32
// app to participate with Windows Action Center.
class NotificationActivator
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          INotificationActivationCallback> {
 public:
  NotificationActivator();

  NotificationActivator& operator=(const NotificationActivator&) = delete;
  NotificationActivator(const NotificationActivator&) = delete;

  // Called when a user interacts with a toast in the Windows action center.
  // For the meaning of the input parameters, see
  // https://msdn.microsoft.com/library/windows/desktop/mt643712.aspx
  IFACEMETHODIMP Activate(LPCWSTR app_user_model_id,
                          LPCWSTR invoked_args,
                          const NOTIFICATION_USER_INPUT_DATA* data,
                          ULONG count) override;

 protected:
  ~NotificationActivator() override;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_NOTIFICATIONS_NOTIFICATION_ACTIVATOR_H_
