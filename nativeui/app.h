// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_APP_H_
#define NATIVEUI_APP_H_

#include <array>
#include <memory>
#include <string>

#include "base/memory/weak_ptr.h"
#include "nativeui/clipboard.h"
#include "nativeui/gfx/color.h"

namespace nu {

class Font;
class MenuBar;

// App wide APIs, this class is managed by State.
class NATIVEUI_EXPORT App {
 public:
  static App* GetCurrent();

  // Deprecated: Return color of a theme component.
  using ThemeColor = Color::Name;
  Color GetColor(ThemeColor name);

  // Return the default GUI font.
  Font* GetDefaultFont();

  // Return clipboard instance.
  Clipboard* GetClipboard(Clipboard::Type type = Clipboard::Type::CopyPaste);

#if defined(OS_MACOSX)
  // Set the application menu.
  void SetApplicationMenu(MenuBar* menu);
  MenuBar* GetApplicationMenu() const;

  // Dock functions.
  void SetDockBadgeLabel(const std::string& text);
  std::string GetDockBadgeLabel() const;
#endif

  base::WeakPtr<App> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  App();
  ~App();

 private:
  friend class State;

  scoped_refptr<Font> default_font_;

  // Array of available clipboards.
  std::array<std::unique_ptr<Clipboard>,
             static_cast<size_t>(Clipboard::Type::Count)> clipboards_;

#if defined(OS_MACOSX)
  scoped_refptr<MenuBar> application_menu_;
#endif

  base::WeakPtrFactory<App> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(App);
};

}  // namespace nu

#endif  // NATIVEUI_APP_H_
