// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WINDOW_H_
#define NATIVEUI_WINDOW_H_

#include <functional>
#include <string>
#include <tuple>

#include "nativeui/container.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect_f.h"

#if defined(OS_WIN)
// Windows headers define macros for these function names which screw with us.
#if defined(IsMaximized)
#undef IsMaximized
#endif
#if defined(IsMinimized)
#undef IsMinimized
#endif
#endif

namespace nu {

class MenuBar;

#if defined(OS_MACOSX)
class Toolbar;
#endif

// The native window.
class NATIVEUI_EXPORT Window : public base::RefCounted<Window> {
 public:
  struct Options {
    // Whether the window has a chrome.
    bool frame = true;
    // Whether the window is transparent.
    bool transparent = false;

#if defined(OS_MACOSX)
    // Show window buttons for the frameless window.
    bool show_traffic_lights = false;
#endif
  };

  explicit Window(const Options& options);

  void Close();

  bool HasFrame() const { return has_frame_; }
  bool IsTransparent() const { return transparent_; }
  void SetHasShadow(bool has);
  bool HasShadow() const;

  void SetContentView(View* view);
  View* GetContentView() const;

  void Center();
  void SetContentSize(const SizeF& size);
  SizeF GetContentSize() const;
  void SetBounds(const RectF& bounds);
  RectF GetBounds() const;

  void SetSizeConstraints(const SizeF& min_size, const SizeF& max_size);
  std::tuple<SizeF, SizeF> GetSizeConstraints() const;
  void SetContentSizeConstraints(const SizeF& min_size, const SizeF& max_size);
  std::tuple<SizeF, SizeF> GetContentSizeConstraints() const;

  void Activate();
  void Deactivate();
  bool IsActive() const;
  void SetVisible(bool visible);
  bool IsVisible() const;
  void SetAlwaysOnTop(bool top);
  bool IsAlwaysOnTop() const;
  void SetFullscreen(bool fullscreen);
  bool IsFullscreen() const;
  void Maximize();
  void Unmaximize();
  bool IsMaximized() const;
  void Minimize();
  void Restore();
  bool IsMinimized() const;

  void SetResizable(bool resizable);
  bool IsResizable() const;
  void SetMaximizable(bool maximizable);
  bool IsMaximizable() const;
  void SetMinimizable(bool minimizable);
  bool IsMinimizable() const;
  void SetMovable(bool movable);
  bool IsMovable() const;

  void SetTitle(const std::string& title);
  std::string GetTitle() const;
  void SetBackgroundColor(Color color);

#if defined(OS_MACOSX)
  void SetToolbar(Toolbar* toolbar);
  Toolbar* GetToolbar() const { return toolbar_.get(); }
  void SetTitleVisible(bool visible);
  bool IsTitleVisible() const;
  void SetFullSizeContentView(bool full);
  bool IsFullSizeContentView() const;
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
  void SetMenuBar(MenuBar* menu_bar);
  MenuBar* GetMenuBar() const { return menu_bar_.get(); }
#endif

  // Get the native window object.
  NativeWindow GetNative() const { return window_; }

  // Internal: Get the yogo config object.
  YGConfigRef GetYogaConfig() const { return yoga_config_; }

  // Events.
  Signal<void(Window*)> on_close;

  // Delegate methods.
  std::function<bool(Window*)> should_close;

 protected:
  virtual ~Window();

 private:
  friend class base::RefCounted<Window>;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit(const Options& options);
  void PlatformDestroy();
  void PlatformSetContentView(View* container);
#if defined(OS_WIN) || defined(OS_LINUX)
  void PlatformSetMenuBar(MenuBar* menu_bar);
#endif

  // Whether window has a native chrome.
  bool has_frame_;

  // Whether window is transparent.
  bool transparent_;

  // Whether there is native shadow.
  bool has_shadow_ = false;

  // The yoga config for window's children.
  YGConfigRef yoga_config_;

#if defined(OS_MACOSX)
  scoped_refptr<Toolbar> toolbar_;
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
  scoped_refptr<MenuBar> menu_bar_;
#endif

  NativeWindow window_ = nullptr;
  scoped_refptr<View> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_WINDOW_H_
