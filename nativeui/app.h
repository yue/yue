// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_APP_H_
#define NATIVEUI_APP_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "nativeui/clipboard.h"
#include "nativeui/gfx/color.h"

namespace nu {

class Font;
class MenuBar;

// App wide APIs, this class is managed by State.
class NATIVEUI_EXPORT App {
 public:
  static App* GetCurrent();

#if defined(OS_MACOSX)
  // Set the application menu.
  void SetApplicationMenu(scoped_refptr<MenuBar> menu);
  MenuBar* GetApplicationMenu() const;

  // Dock functions.
  void SetDockBadgeLabel(const std::string& text);
  std::string GetDockBadgeLabel() const;

  // Activations.
  void Activate(bool force);
  void Deactivate();
  bool IsActive() const;

  // Activation policy.
  enum class ActivationPolicy {
    Regular,
    Accessory,
    Prohibited,
  };
  void SetActivationPolicy(ActivationPolicy policy);
  ActivationPolicy GetActivationPolicy() const;
#endif

#if defined(OS_WIN)
  bool IsRunningAsUWP() const;
  void SetAppUserModelID(const std::string& id);
  std::wstring GetAppUserModelID() const;
  std::wstring GetNameW() const;

  struct ShortcutOptions {
    base::Optional<std::wstring> arguments;
    base::Optional<std::wstring> description;
    base::Optional<base::FilePath> working_dir;
  };
  bool CreateStartMenuShortcut(const ShortcutOptions& options);
  base::FilePath GetStartMenuShortcutPath() const;
#endif

  base::WeakPtr<App> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  App();
  ~App();

 private:
  friend class State;

#if defined(OS_MACOSX)
  scoped_refptr<MenuBar> application_menu_;
#endif

  base::WeakPtrFactory<App> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(App);
};

}  // namespace nu

#endif  // NATIVEUI_APP_H_
