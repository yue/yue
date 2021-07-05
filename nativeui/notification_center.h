// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_NOTIFICATION_CENTER_H_
#define NATIVEUI_NOTIFICATION_CENTER_H_

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace base {
class FilePath;
}

namespace nu {

class NATIVEUI_EXPORT NotificationCenter : SignalDelegate {
 public:
  ~NotificationCenter() override;

  static NotificationCenter* GetCurrent();

  void Clear();

#if defined(OS_WIN)
  struct COMServerOptions {
    bool write_registry = true;
    std::wstring arguments;
    base::Optional<std::wstring> toast_activator_clsid;
  };
  bool SetCOMServerOptions(COMServerOptions options);
  bool RegisterCOMServer();
  void RemoveCOMServerFromRegistry();

  const std::wstring& GetToastActivatorCLSID();
  ::GUID GetRawToastActivatorCLSID();
#endif

  NativeNotificationCenter GetNative() const { return center_; }

  base::WeakPtr<NotificationCenter> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  // Events.
  Signal<void(const std::string&)> on_notification_show;
  Signal<void(const std::string&)> on_notification_close;
  Signal<void(const std::string&)> on_notification_click;
  Signal<void(const std::string&)> on_notification_action;
#if defined(OS_MAC) || defined(OS_WIN)
  Signal<void(const std::string&, const std::string&)> on_notification_reply;
#endif

  // Delegate methods.
#if defined(OS_WIN)
  struct InputData {
    std::wstring key;
    std::wstring value;
  };
  std::function<bool(const std::wstring&,
                     const std::wstring&,
                     const std::vector<InputData>&)> toast_activate;
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

  base::Optional<::GUID> toast_activator_clsid_;
  std::wstring toast_activator_clsid_string_;
#endif

  NativeNotificationCenter center_ = nullptr;

  base::WeakPtrFactory<NotificationCenter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(NotificationCenter);
};

}  // namespace nu

#endif  // NATIVEUI_NOTIFICATION_CENTER_H_
