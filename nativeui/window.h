// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WINDOW_H_
#define NATIVEUI_WINDOW_H_

#include <functional>
#include <string>
#include <tuple>
#include <vector>

#include "nativeui/container.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect_f.h"

#if defined(OS_WIN)
#include "base/win/scoped_gdi_object.h"
#endif

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

#if defined(OS_MAC)
class Toolbar;
#endif

// The native window.
class NATIVEUI_EXPORT Window : public base::RefCounted<Window>,
                               public Responder<Window> {
 public:
  struct Options {
    // Whether the window has a chrome.
    bool frame = true;
    // Whether the window is transparent.
    bool transparent = false;

#if defined(OS_LINUX) || defined(OS_WIN)
    // The window can not be activated.
    bool no_activate = false;
#endif
#if defined(OS_MAC)
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

  void SetContentView(scoped_refptr<View> view);
  View* GetContentView() const;

  void Center();
  void SetContentSize(const SizeF& size);
  SizeF GetContentSize() const;
  RectF GetContentBounds() const;
  void SetBounds(const RectF& bounds);
  RectF GetBounds() const;

  RectF ContentBoundsToWindowBounds(const RectF& bounds) const;
  RectF WindowBoundsToContentBounds(const RectF& bounds) const;

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

  void SetCapture();
  void ReleaseCapture();
  bool HasCapture() const;

#if defined(OS_MAC)
  void SetToolbar(scoped_refptr<Toolbar> toolbar);
  Toolbar* GetToolbar() const { return toolbar_.get(); }
  void SetTitleVisible(bool visible);
  bool IsTitleVisible() const;
  void SetFullSizeContentView(bool full);
  bool IsFullSizeContentView() const;
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
  void SetSkipTaskbar(bool skip);
  void SetIcon(scoped_refptr<Image> icon);
  void SetMenuBar(scoped_refptr<MenuBar> menu_bar);
  MenuBar* GetMenuBar() const { return menu_bar_.get(); }
#endif

  Window* GetParentWindow() const { return parent_; }
  void AddChildWindow(scoped_refptr<Window> child);
  void RemoveChildWindow(Window* child);
  std::vector<Window*> GetChildWindows() const;

  // Internal: Destroy all child windows and notify window is closed.
  void NotifyWindowClosed();

  // Get the native window object.
  NativeWindow GetNative() const { return window_; }

  // Internal: Get the yogo config object.
  YGConfigRef GetYogaConfig() const { return yoga_config_; }

  // Events.
  Signal<void(Window*)> on_close;
  Signal<void(Window*)> on_focus;
  Signal<void(Window*)> on_blur;
  Signal<void(Window*)> on_capture_lost;

  // Delegate methods.
  std::function<bool(Window*)> should_close;

 protected:
  ~Window() override;

 private:
  friend class base::RefCounted<Window>;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit(const Options& options);
  void PlatformDestroy();
  void PlatformSetContentView(View* container);
#if defined(OS_WIN) || defined(OS_LINUX)
  void PlatformSetIcon(Image* icon);
  void PlatformSetMenuBar(MenuBar* menu_bar);
#endif
  void PlatformAddChildWindow(Window* child);
  void PlatformRemoveChildWindow(Window* child);

  // Whether window has a native chrome.
  bool has_frame_;

  // Whether window is transparent.
  bool transparent_;

  // The yoga config for window's children.
  YGConfigRef yoga_config_;

  // Whehter window has been closed.
  bool is_closed_ = false;

#if defined(OS_MAC)
  scoped_refptr<Toolbar> toolbar_;
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
  scoped_refptr<Image> icon_;
#if defined(OS_WIN)
  base::win::ScopedHICON hicon_;
#endif
  scoped_refptr<MenuBar> menu_bar_;
#endif

  Window* parent_ = nullptr;
  std::vector<scoped_refptr<Window>> child_windows_;

  NativeWindow window_ = nullptr;
  scoped_refptr<View> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_WINDOW_H_
