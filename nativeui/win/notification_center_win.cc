// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/notification_center.h"

#include <utility>

#include "base/base_paths.h"
#include "base/hash/hash.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "base/path_service.h"
#include "base/strings/string_util_win.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"
#include "base/win/scoped_co_mem.h"
#include "nativeui/app.h"
#include "nativeui/state.h"
#include "nativeui/win/notifications/com_server_module.h"
#include "nativeui/win/notifications/winrt_util.h"

namespace nu {

void NotificationCenter::Clear() {
  if (!State::GetCurrent()->InitializeWinRT())
    return;

  mswr::ComPtr<winui::Notifications::IToastNotificationManagerStatics>
      toast_manager;
  HRESULT hr = CreateActivationFactory(
      RuntimeClass_Windows_UI_Notifications_ToastNotificationManager,
      IID_PPV_ARGS(&toast_manager));
  if (FAILED(hr)) {
    DLOG(ERROR) << "Unable to create the ToastNotificationManager "
                << std::hex << hr;
    return;
  }

  mswr::ComPtr<winui::Notifications::IToastNotificationManagerStatics2>
      toast_manager2;
  hr = toast_manager.As(&toast_manager2);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Failed to get IToastNotificationManagerStatics2 "
                << std::hex << hr;
    return;
  }

  mswr::ComPtr<winui::Notifications::IToastNotificationHistory>
      notification_history;
  hr = toast_manager2->get_History(&notification_history);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Failed to get IToastNotificationHistory " << std::hex
                << hr;
    return;
  }

  App* app = App::GetCurrent();
  if (app->IsRunningAsUWP()) {
    notification_history->Clear();
  } else {
    std::wstring app_user_model_id = app->GetAppUserModelID();
    notification_history->ClearWithId(
        ScopedHString::Create(app_user_model_id).get());
  }
}

bool NotificationCenter::SetCOMServerOptions(COMServerOptions options) {
  if (options.toast_activator_clsid) {
    if (toast_activator_clsid_) {
      LOG(ERROR) << "The CLSID has already been set, please check if a "
                    "listener of NotificationCenter has been installed, or "
                    "if App::CreateStartMenuShortcut has been called.";
      return false;
    }
    ::GUID guid;
    if (FAILED(::CLSIDFromString(options.toast_activator_clsid->c_str(),
                                 &guid))) {
      LOG(ERROR) << "Invalid CLSID: " << *options.toast_activator_clsid;
      return false;
    }
    toast_activator_clsid_.emplace(std::move(guid));
    toast_activator_clsid_string_ = std::move(*options.toast_activator_clsid);
  }
  options_ = std::move(options);
  return true;
}

bool NotificationCenter::RegisterCOMServer() {
  base::FilePath exe_path;
  if (!base::PathService::Get(base::FILE_EXE, &exe_path)) {
    DLOG(ERROR) << "Failed to get FILE_EXE path";
    return false;
  }

  std::wstring key = L"SOFTWARE\\Classes\\CLSID\\" + GetToastActivatorCLSID();
  std::wstring class_name = base::UTF8ToWide(App::GetCurrent()->GetName()) +
                            L" ToastActivator Class";
  LONG result =
      base::win::RegKey(HKEY_CURRENT_USER, key.c_str(), KEY_ALL_ACCESS)
          .WriteValue(L"", class_name.c_str());
  if (result != ERROR_SUCCESS) {
    SetLastError(result);
    PLOG(ERROR) << "Failed to write to " << key;
    return false;
  }

  std::wstring server_key = key + L"\\LocalServer32";
  std::wstring value = L"\"" + exe_path.value() + L"\" " + options_.arguments;
  result =
      base::win::RegKey(HKEY_CURRENT_USER, server_key.c_str(), KEY_ALL_ACCESS)
          .WriteValue(L"", value.c_str());
  if (result != ERROR_SUCCESS) {
    SetLastError(result);
    PLOG(ERROR) << "Failed to write to " << server_key;
    return false;
  }

  return true;
}

void NotificationCenter::RemoveCOMServerFromRegistry() {
  base::win::RegKey(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\CLSID\\",
                    KEY_ALL_ACCESS)
      .DeleteKey(GetToastActivatorCLSID().c_str());
}

const std::wstring& NotificationCenter::GetToastActivatorCLSID() {
  if (!toast_activator_clsid_)
    GetRawToastActivatorCLSID();
  return toast_activator_clsid_string_;
}

::GUID NotificationCenter::GetRawToastActivatorCLSID() {
  if (!toast_activator_clsid_) {
    // Generate one from AppUserModelID if not set.
    ::GUID clsid;
    uint32_t h = base::Hash(
        base::as_u16cstr(App::GetCurrent()->GetAppUserModelID()));
    for (size_t i = 0; i < sizeof(GUID); i += sizeof(uint32_t))
      memcpy(reinterpret_cast<char*>(&clsid) + i, &h, sizeof(uint32_t));
    base::win::ScopedCoMem<OLECHAR> clsid_string;
    if (FAILED(::StringFromCLSID(clsid, &clsid_string)))
      NOTREACHED() << "StringFromCLSID failed";
    toast_activator_clsid_.emplace(std::move(clsid));
    toast_activator_clsid_string_ = clsid_string.get();
  }
  return *toast_activator_clsid_;
}

void NotificationCenter::PlatformInit() {
  if (!State::GetCurrent()->InitializeWinRT())
    return;

  State::GetCurrent()->InitializeCOM();
  if (options_.write_registry)
    RegisterCOMServer();

  center_ = new ComServerModule(GetRawToastActivatorCLSID());
  center_->RegisterClassObjects();
}

void NotificationCenter::PlatformDestroy() {
  center_->UnregisterClassObjects();
  delete center_;
}

}  // namespace nu
