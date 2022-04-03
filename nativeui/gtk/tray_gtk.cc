// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/tray.h"

#include <dlfcn.h>
#include <gtk/gtk.h>

#include "base/check.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "nativeui/app.h"
#include "nativeui/gfx/image.h"
#include "nativeui/menu.h"

namespace nu {

namespace {

typedef enum {
  APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
  APP_INDICATOR_CATEGORY_COMMUNICATIONS,
  APP_INDICATOR_CATEGORY_SYSTEM_SERVICES,
  APP_INDICATOR_CATEGORY_HARDWARE,
  APP_INDICATOR_CATEGORY_OTHER
} AppIndicatorCategory;

typedef enum {
  APP_INDICATOR_STATUS_PASSIVE,
  APP_INDICATOR_STATUS_ACTIVE,
  APP_INDICATOR_STATUS_ATTENTION
} AppIndicatorStatus;

typedef AppIndicator* (*app_indicator_new_func)(const gchar* id,
                                                const gchar* icon_name,
                                                AppIndicatorCategory category);
typedef AppIndicator* (*app_indicator_new_with_path_func)(
    const gchar* id,
    const gchar* icon_name,
    AppIndicatorCategory category,
    const gchar* icon_theme_path);
typedef void (*app_indicator_set_status_func)(AppIndicator* self,
                                              AppIndicatorStatus status);
typedef void (*app_indicator_set_label_func)(AppIndicator* self,
                                             const gchar* label,
                                             const gchar* labelwhat);
typedef void (*app_indicator_set_attention_icon_full_func)(
    AppIndicator* self,
    const gchar* icon_name,
    const gchar* icon_desc);
typedef void (*app_indicator_set_menu_func)(AppIndicator* self, GtkMenu* menu);
typedef void (*app_indicator_set_icon_full_func)(AppIndicator* self,
                                                 const gchar* icon_name,
                                                 const gchar* icon_desc);
typedef void (*app_indicator_set_icon_theme_path_func)(
    AppIndicator* self,
    const gchar* icon_theme_path);

bool g_attempted_load = false;
bool g_opened = false;

// Retrieved functions from libappindicator.
app_indicator_new_func app_indicator_new = nullptr;
app_indicator_new_with_path_func app_indicator_new_with_path = nullptr;
app_indicator_set_status_func app_indicator_set_status = nullptr;
app_indicator_set_label_func app_indicator_set_label = nullptr;
app_indicator_set_attention_icon_full_func
    app_indicator_set_attention_icon_full = nullptr;
app_indicator_set_menu_func app_indicator_set_menu = nullptr;
app_indicator_set_icon_full_func app_indicator_set_icon_full = nullptr;
app_indicator_set_icon_theme_path_func app_indicator_set_icon_theme_path =
    nullptr;

void EnsureMethodsLoaded() {
  if (g_attempted_load)
    return;
  g_attempted_load = true;

  void* indicator_lib = nullptr;

  // These include guards might be unnecessary, but let's keep them as a
  // precaution since using gtk2 and gtk3 symbols in the same process is
  // explicitly unsupported.
#if GTK_MAJOR_VERSION == 2
  if (!indicator_lib)
    indicator_lib = dlopen("libappindicator.so", RTLD_LAZY);
  if (!indicator_lib)
    indicator_lib = dlopen("libappindicator.so.1", RTLD_LAZY);
  if (!indicator_lib)
    indicator_lib = dlopen("libappindicator.so.0", RTLD_LAZY);
#endif

#if GTK_MAJOR_VERSION == 3
  if (!indicator_lib)
    indicator_lib = dlopen("libappindicator3.so", RTLD_LAZY);
  if (!indicator_lib)
    indicator_lib = dlopen("libappindicator3.so.1", RTLD_LAZY);
#endif

  if (!indicator_lib)
    return;

  g_opened = true;

  app_indicator_new = reinterpret_cast<app_indicator_new_func>(
      dlsym(indicator_lib, "app_indicator_new"));
  app_indicator_new_with_path =
      reinterpret_cast<app_indicator_new_with_path_func>(
          dlsym(indicator_lib, "app_indicator_new_with_path"));
  app_indicator_set_status = reinterpret_cast<app_indicator_set_status_func>(
      dlsym(indicator_lib, "app_indicator_set_status"));
  app_indicator_set_label = reinterpret_cast<app_indicator_set_label_func>(
      dlsym(indicator_lib, "app_indicator_set_label"));
  app_indicator_set_attention_icon_full =
      reinterpret_cast<app_indicator_set_attention_icon_full_func>(
          dlsym(indicator_lib, "app_indicator_set_attention_icon_full"));
  app_indicator_set_menu = reinterpret_cast<app_indicator_set_menu_func>(
      dlsym(indicator_lib, "app_indicator_set_menu"));
  app_indicator_set_icon_full =
      reinterpret_cast<app_indicator_set_icon_full_func>(
          dlsym(indicator_lib, "app_indicator_set_icon_full"));
  app_indicator_set_icon_theme_path =
      reinterpret_cast<app_indicator_set_icon_theme_path_func>(
          dlsym(indicator_lib, "app_indicator_set_icon_theme_path"));
}

// Generate ID based on the exe name.
std::string GenerateAppIndicatorId() {
  static int count = 0;
  return base::StringPrintf("%s-appindicator-%d",
                            App::GetCurrent()->GetName().c_str(),
                            count++);
}

}  // namespace

Tray::Tray(scoped_refptr<Image> icon)
    : id_(GenerateAppIndicatorId()) {
  EnsureMethodsLoaded();
  if (!g_opened) {
    LOG(ERROR) << "Unable to load libappindicator";
    return;
  }

  std::string name = WriteImage(icon.get());
  if (name.empty()) {
    LOG(ERROR) << "Unable to save tray icon to temp dir";
    return;
  }

  tray_ = app_indicator_new_with_path(id_.c_str(), name.c_str(),
                                      APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
                                      temp_dir_.GetPath().value().c_str());
  app_indicator_set_status(tray_, APP_INDICATOR_STATUS_ACTIVE);
}

Tray::~Tray() {
  Remove();
}

void Tray::Remove() {
  if (!tray_)
    return;
  app_indicator_set_status(tray_, APP_INDICATOR_STATUS_PASSIVE);
  g_object_unref(tray_);
  tray_ = nullptr;
}

void Tray::SetTitle(const std::string& title) {
  if (!tray_)
    return;
  app_indicator_set_label(tray_, title.c_str(), title.c_str());
}

void Tray::SetImage(scoped_refptr<Image> icon) {
  if (!tray_)
    return;

  std::string name = WriteImage(icon.get());
  if (name.empty()) {
    LOG(ERROR) << "Unable to save tray icon to temp dir";
    return;
  }

  app_indicator_set_icon_theme_path(tray_, temp_dir_.GetPath().value().c_str());
  app_indicator_set_icon_full(tray_, name.c_str(), "icon");
}

void Tray::SetMenu(scoped_refptr<Menu> menu) {
  if (!tray_)
    return;
  app_indicator_set_menu(tray_, GTK_MENU(menu->GetNative()));
  menu_ = std::move(menu);
}

std::string Tray::WriteImage(Image* icon) {
  std::ignore = temp_dir_.Delete();
  if (!temp_dir_.CreateUniqueTempDir())
    return "";

  std::string name = base::StringPrintf("%s-%d",
                                        id_.c_str(), icon_change_count_++);
  base::FilePath icon_path = temp_dir_.GetPath().Append(name + ".png");
  if (!icon->WriteToFile("png", icon_path))
    return "";
  return name;
}

}  // namespace nu
