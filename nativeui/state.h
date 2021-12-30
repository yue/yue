// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_STATE_H_
#define NATIVEUI_STATE_H_

#include <array>
#include <memory>

#include "base/memory/ref_counted.h"
#include "nativeui/app.h"

typedef struct YGConfig *YGConfigRef;

#if defined(OS_WIN)
namespace base {
class ScopedNativeLibrary;
namespace win {
class ScopedCOMInitializer;
}
}
#endif

namespace nu {

class Appearance;
class Font;
class NotificationCenter;
class Screen;

#if defined(OS_WIN)
class ClassRegistrar;
class GdiplusHolder;
class NativeTheme;
class SubwinHolder;
class ScopedOleInitializer;
class TrayHost;
class TimerHost;
#elif defined(OS_LINUX)
class GtkTheme;
#endif

class NATIVEUI_EXPORT State {
 public:
  State();
  ~State();

  static State* GetCurrent();

  // Internal: Get the state created for the main thread.
  static State* GetMain();

  // Return the instance of App.
  App* GetApp() { return &app_; }

  // Internal classes.
#if defined(OS_WIN)
  void InitializeCOM();
  bool InitializeWinRT();
  bool InitWebView2Loader();
  base::ScopedNativeLibrary* GetWebView2Loader();
  HWND GetSubwinHolder();
  ClassRegistrar* GetClassRegistrar();
  NativeTheme* GetNativeTheme();
  TrayHost* GetTrayHost();
  TimerHost* GetTimerHost();
  UINT GetNextCommandID();
#elif defined(OS_LINUX)
  GtkTheme* GetGtkTheme();
#endif

  // Internal: Return the clipboards.
  Clipboard* GetClipboard(Clipboard::Type type);

  // Internal: Return the screen object
  Screen* GetScreen();

  // Internal: Return the appearance object
  Appearance* GetAppearance();

  // Internal: Return the notificationCenter object
  NotificationCenter* GetNotificationCenter();

  // Internal: Return the default font.
  scoped_refptr<Font>& default_font() { return default_font_; }

  // Internal: Return the default yoga config.
  YGConfigRef yoga_config() const { return yoga_config_; }

 private:
  void PlatformInit();

#if defined(OS_WIN)
  std::unique_ptr<base::win::ScopedCOMInitializer> com_initializer_;
  std::unique_ptr<base::ScopedNativeLibrary> webview2_loader_;
  std::unique_ptr<ScopedOleInitializer> ole_initializer_;
  std::unique_ptr<GdiplusHolder> gdiplus_holder_;
  std::unique_ptr<ClassRegistrar> class_registrar_;
  std::unique_ptr<SubwinHolder> subwin_holder_;
  std::unique_ptr<NativeTheme> native_theme_;
  std::unique_ptr<TrayHost> tray_host_;
  std::unique_ptr<TimerHost> timer_host_;

  // Next ID for custom WM_COMMAND items, the number came from:
  // https://msdn.microsoft.com/en-us/library/11861byt.aspx
  UINT next_command_id_ = 0x8000;
#endif

#if defined(OS_LINUX)
  std::unique_ptr<GtkTheme> gtk_theme_;
#endif

  // Array of available clipboards.
  std::array<std::unique_ptr<Clipboard>,
             static_cast<size_t>(Clipboard::Type::Count)> clipboards_;

  std::unique_ptr<Screen> screen_;
  std::unique_ptr<Appearance> appearance_;
  std::unique_ptr<NotificationCenter> notification_center_;
  scoped_refptr<Font> default_font_;

  // The app instance.
  App app_;

  YGConfigRef yoga_config_;
};

}  // namespace nu

#endif  // NATIVEUI_STATE_H_
