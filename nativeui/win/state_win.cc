// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#include <shellscalingapi.h>

#include "base/base_paths.h"
#include "base/path_service.h"
#include "base/scoped_native_library.h"
#include "base/win/scoped_com_initializer.h"
#include "base/win/windows_version.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/gfx/win/native_theme.h"
#include "nativeui/win/util/class_registrar.h"
#include "nativeui/win/util/gdiplus_holder.h"
#include "nativeui/win/util/scoped_ole_initializer.h"
#include "nativeui/win/util/subwin_holder.h"
#include "nativeui/win/util/tray_host.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

namespace {

// Win8.1 supports monitor-specific DPI scaling.
bool SetProcessDpiAwarenessWrapper(PROCESS_DPI_AWARENESS value) {
  typedef HRESULT(WINAPI *SetProcessDpiAwarenessPtr)(PROCESS_DPI_AWARENESS);
  SetProcessDpiAwarenessPtr set_process_dpi_awareness_func =
      reinterpret_cast<SetProcessDpiAwarenessPtr>(
          GetProcAddress(GetModuleHandleA("user32.dll"),
                         "SetProcessDpiAwarenessInternal"));
  if (set_process_dpi_awareness_func) {
    HRESULT hr = set_process_dpi_awareness_func(value);
    if (SUCCEEDED(hr)) {
      VLOG(1) << "SetProcessDpiAwareness succeeded.";
      return true;
    } else if (hr == E_ACCESSDENIED) {
      LOG(ERROR) << "Access denied error from SetProcessDpiAwareness. "
          "Function called twice, or manifest was used.";
    }
  }
  return false;
}

// This function works for Windows Vista through Win8. Win8.1 must use
// SetProcessDpiAwareness[Wrapper].
BOOL SetProcessDPIAwareWrapper() {
  typedef BOOL(WINAPI *SetProcessDPIAwarePtr)(VOID);
  SetProcessDPIAwarePtr set_process_dpi_aware_func =
      reinterpret_cast<SetProcessDPIAwarePtr>(
      GetProcAddress(GetModuleHandleA("user32.dll"),
                      "SetProcessDPIAware"));
  return set_process_dpi_aware_func &&
    set_process_dpi_aware_func();
}

void EnableHighDPISupport() {
  // Enable per-monitor DPI for Win10 or above instead of Win8.1 since Win8.1
  // does not have EnableChildWindowDpiMessage, necessary for correct non-client
  // area scaling across monitors.
  bool allowed_platform = base::win::GetVersion() >= base::win::VERSION_WIN10;
  PROCESS_DPI_AWARENESS process_dpi_awareness =
      allowed_platform ? PROCESS_PER_MONITOR_DPI_AWARE
                       : PROCESS_SYSTEM_DPI_AWARE;
  if (!SetProcessDpiAwarenessWrapper(process_dpi_awareness)) {
    SetProcessDPIAwareWrapper();
  }
}

}  // namespace

void State::PlatformInit() {
  EnableHighDPISupport();

  YGConfigSetPointScaleFactor(yoga_config(), GetScaleFactor());

  // Initialize Common Controls.
  INITCOMMONCONTROLSEX config;
  config.dwSize = sizeof(config);
  config.dwICC = ICC_WIN95_CLASSES;
  ::InitCommonControlsEx(&config);

  gdiplus_holder_.reset(new GdiplusHolder);
}

void State::InitializeCOM() {
  if (!com_initializer_) {
    com_initializer_.reset(new base::win::ScopedCOMInitializer);
    ole_initializer_.reset(new ScopedOleInitializer);
  }
}

bool State::InitWebView2Loader() {
  if (!webview2_loader_) {
    // First try to do a global search.
    webview2_loader_.reset(
        new base::ScopedNativeLibrary(base::FilePath(L"WebView2Loader.dll")));
    // Then search from dir the contains current module.
    if (!webview2_loader_->is_valid()) {
      base::FilePath module_path;
      if (PathService::Get(base::FILE_MODULE, &module_path)) {
        auto dll_path = module_path.DirName().Append(L"WebView2Loader.dll");
        webview2_loader_.reset(new base::ScopedNativeLibrary(dll_path));
      }
    }
  }
  return webview2_loader_->is_valid();
}

HWND State::GetSubwinHolder() {
  if (!subwin_holder_)
    subwin_holder_.reset(new SubwinHolder);
  return subwin_holder_->hwnd();
}

ClassRegistrar* State::GetClassRegistrar() {
  if (!class_registrar_)
    class_registrar_.reset(new ClassRegistrar);
  return class_registrar_.get();
}

NativeTheme* State::GetNativeTheme() {
  if (!native_theme_)
    native_theme_.reset(new NativeTheme);
  return native_theme_.get();
}

TrayHost* State::GetTrayHost() {
  if (!tray_host_)
    tray_host_.reset(new TrayHost);
  return tray_host_.get();
}

UINT State::GetNextCommandID() {
  return next_command_id_++;
}

}  // namespace nu
