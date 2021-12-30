// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_NOTIFICATION_CENTER_H_
#define NATIVEUI_NOTIFICATION_CENTER_H_

#include <set>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace base {
class FilePath;
}

namespace nu {

class Notification;

class NATIVEUI_EXPORT NotificationCenter : SignalDelegate {
 public:
  ~NotificationCenter() override;

  static NotificationCenter* GetCurrent();

  void Clear();

#if defined(OS_WIN)
  struct COMServerOptions {
    bool write_registry = true;
    std::wstring arguments;
    absl::optional<std::wstring> toast_activator_clsid;
  };
  bool SetCOMServerOptions(COMServerOptions options);
  bool RegisterCOMServer();
  void RemoveCOMServerFromRegistry();

  const std::wstring& GetToastActivatorCLSID();
  ::GUID GetRawToastActivatorCLSID();
#endif

#if defined(OS_LINUX)
  // Internal: Notification managements.
  void AddNotification(Notification* notification);
  void RemoveNotification(Notification* notification);
  Notification* FindNotification(uint32_t dbus_id) const;
#endif

  NativeNotificationCenter GetNative();

  base::WeakPtr<NotificationCenter> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  // Events.
  Signal<void(const std::string&)> on_notification_show;
  Signal<void(const std::string&)> on_notification_close;
  Signal<void(const std::string&)> on_notification_click;
  Signal<void(const std::string&)> on_notification_action;
#if defined(OS_MAC) || defined(OS_WIN)
  Signal<void(const std::string&, std::string)> on_notification_reply;
#endif
#if defined(OS_WIN)
  struct InputData {
    std::wstring key;
    std::wstring value;
  };
  Signal<bool(std::wstring,
              std::wstring,
              std::vector<InputData>)> on_toast_activate;
#endif

 protected:
  NotificationCenter();

  // SignalDelegate:
  void OnConnect(int identifier) override;

 private:
  friend class State;

  void PlatformInit();
  void PlatformDestroy();

#if defined(OS_WIN)
  COMServerOptions options_;

  absl::optional<::GUID> toast_activator_clsid_;
  std::wstring toast_activator_clsid_string_;
#endif

#if defined(OS_LINUX)
  // Stores the notifications that have been shown.
  std::set<scoped_refptr<Notification>> notifications_;

  // Used to disconnect from "g-signal" during destruction.
  unsigned long proxy_signal_handler_ = 0;  // NOLINT
#endif

  NativeNotificationCenter center_ = nullptr;

  base::WeakPtrFactory<NotificationCenter> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_NOTIFICATION_CENTER_H_
