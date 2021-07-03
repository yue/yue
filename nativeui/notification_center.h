// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_NOTIFICATION_CENTER_H_
#define NATIVEUI_NOTIFICATION_CENTER_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class NATIVEUI_EXPORT NotificationCenter : SignalDelegate {
 public:
  ~NotificationCenter() override;

  static NotificationCenter* GetCurrent();

  void Clear();
  NativeNotificationCenter GetNative() const { return center_; }

  base::WeakPtr<NotificationCenter> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  // Events.
  Signal<void(const std::string&)> on_notification_show;
  Signal<void(const std::string&)> on_notification_close;
  Signal<void(const std::string&)> on_notification_click;
  Signal<void(const std::string&, const std::string&)> on_notification_action;
#if defined(OS_MAC)
  Signal<void(const std::string&, const std::string&)> on_notification_reply;
#endif

 protected:
  NotificationCenter();

  // SignalDelegate:
  void OnConnect(int identifier) override;

 private:
  friend class State;

  void PlatformInit();
  void PlatformDestroy();

  NativeNotificationCenter center_ = nullptr;

  base::WeakPtrFactory<NotificationCenter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(NotificationCenter);
};

}  // namespace nu

#endif  // NATIVEUI_NOTIFICATION_CENTER_H_
