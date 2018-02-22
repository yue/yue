// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_APP_H_
#define NATIVEUI_APP_H_

#include <unordered_map>

#include "base/memory/weak_ptr.h"
#include "nativeui/gfx/color.h"

namespace nu {

class Font;
class MenuBar;

// App wide APIs, this class is managed by State.
class NATIVEUI_EXPORT App {
 public:
  static App* GetCurrent();

  // Available theme names of colors.
  enum class ThemeColor {
    Text,
    DisabledText,
  };

  // Return color of a theme component.
  Color GetColor(ThemeColor name);

  // Return the default GUI font.
  Font* GetDefaultFont();

#if defined(OS_MACOSX)
  // Set the application menu.
  void SetApplicationMenu(MenuBar* menu);
  MenuBar* GetApplicationMenu() const;
#endif

  base::WeakPtr<App> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  App();
  ~App();

 private:
  friend class State;

  Color PlatformGetColor(ThemeColor name);

  // Cached theme colors.
  std::unordered_map<int, Color> theme_colors_;

  scoped_refptr<Font> default_font_;

#if defined(OS_MACOSX)
  scoped_refptr<MenuBar> application_menu_;
#endif

  base::WeakPtrFactory<App> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(App);
};

}  // namespace nu

#endif  // NATIVEUI_APP_H_
