// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include <appmodel.h>
#include <shobjidl_core.h>

#include "base/base_paths.h"
#include "base/base_paths_win.h"
#include "base/file_version_info.h"
#include "base/logging.h"
#include "base/optional.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/shortcut.h"
#include "nativeui/notification_center.h"
#include "nativeui/state.h"

namespace nu {

bool App::IsRunningAsUWP() const {
  static base::Optional<bool> result;
  if (!result) {
    result.emplace(false);
    auto get_package_family_name =
        reinterpret_cast<decltype(&GetPackageFamilyName)>(
            ::GetProcAddress(::GetModuleHandleW(L"Kernel32.dll"),
                             "GetPackageFamilyName"));
    if (get_package_family_name) {
      UINT32 length;
      wchar_t name[PACKAGE_FAMILY_NAME_MAX_LENGTH + 1];
      LONG r = get_package_family_name(GetCurrentProcess(), &length, name);
      result.emplace(r == ERROR_SUCCESS);
    }
  }
  return *result;
}

void App::SetAppUserModelID(const std::string& id) {
  ::SetCurrentProcessExplicitAppUserModelID(base::UTF8ToUTF16(id).c_str());
}

std::wstring App::GetAppUserModelID() const {
  PWSTR id;
  if (FAILED(::GetCurrentProcessExplicitAppUserModelID(&id)))
    return std::wstring();
  std::wstring result(id);
  ::CoTaskMemFree(id);
  return result;
}

std::wstring App::GetNameW() const {
  // Get the application name with order:
  // 1. The product name specified in exe file.
  // 2. The name of the exe file.
  // 3. "Yue"
  std::wstring name;
  base::FilePath path;
  if (base::PathService::Get(base::FILE_EXE, &path)) {
    auto info = FileVersionInfo::CreateFileVersionInfo(path);
    if (info && !info->product_name().empty())
      name = info->product_name();
    else
      name = path.BaseName().RemoveExtension().value();
  }
  return name.empty() ? L"Yue" : name;
}

bool App::CreateStartMenuShortcut(
    const ShortcutOptions& options) {
  base::FilePath exe_path;
  if (!base::PathService::Get(base::FILE_EXE, &exe_path)) {
    LOG(ERROR) << "Failed to get FILE_EXE path";
    return false;
  }

  std::wstring app_user_model_id = GetAppUserModelID();
  if (app_user_model_id.empty()) {
    LOG(ERROR) << "AppUserModelID is not set";
    return false;
  }

  base::win::ShortcutProperties properties;
  properties.set_target(exe_path);
  properties.set_app_id(app_user_model_id);
  properties.set_toast_activator_clsid(
      NotificationCenter::GetCurrent()->GetRawToastActivatorCLSID());
  if (options.arguments)
    properties.set_arguments(*options.arguments);
  if (options.description)
    properties.set_description(*options.description);
  if (options.working_dir)
    properties.set_working_dir(*options.working_dir);

  State::GetCurrent()->InitializeCOM();
  return base::win::CreateOrUpdateShortcutLink(
      GetStartMenuShortcutPath(),
      properties,
      base::win::SHORTCUT_CREATE_ALWAYS);
}

base::FilePath App::GetStartMenuShortcutPath() const {
  std::wstring name = GetNameW() + L".LNK";
  base::FilePath start_menu_path;
  if (!base::PathService::Get(base::DIR_START_MENU, &start_menu_path)) {
    DLOG(ERROR) << "Failed to get DIR_START_MENU path";
    return base::FilePath(name);
  }
  return start_menu_path.Append(name);
}

}  // namespace nu
