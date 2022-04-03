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
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/shortcut.h"
#include "nativeui/notification_center.h"
#include "nativeui/state.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace nu {

void App::SetID(std::string id) {
  ::SetCurrentProcessExplicitAppUserModelID(base::UTF8ToWide(id).c_str());
}

std::string App::GetID() const {
  return base::WideToUTF8(GetAppUserModelID());
}

bool App::IsRunningAsUWP() const {
  static absl::optional<bool> result;
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

std::wstring App::GetAppUserModelID() const {
  PWSTR id;
  if (FAILED(::GetCurrentProcessExplicitAppUserModelID(&id)))
    return std::wstring();
  std::wstring result(id);
  ::CoTaskMemFree(id);
  return result;
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
      base::win::ShortcutOperation::kCreateAlways);
}

base::FilePath App::GetStartMenuShortcutPath() const {
  std::wstring name = base::UTF8ToWide(GetName()) + L".LNK";
  base::FilePath start_menu_path;
  if (!base::PathService::Get(base::DIR_START_MENU, &start_menu_path)) {
    DLOG(ERROR) << "Failed to get DIR_START_MENU path";
    return base::FilePath(name);
  }
  return start_menu_path.Append(name);
}

bool App::PlatformGetName(std::string* name) const {
  // Get the product name specified in exe file.
  base::FilePath path;
  if (!base::PathService::Get(base::FILE_EXE, &path))
    return false;
  auto info = FileVersionInfo::CreateFileVersionInfo(path);
  if (!info || info->product_name().empty())
    return false;
  *name = base::UTF16ToUTF8(info->product_name());
  return true;
}

}  // namespace nu
