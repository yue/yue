// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WINDOW_H_
#define NATIVEUI_WINDOW_H_

#include "nativeui/container.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect_f.h"

namespace nu {

class MenuBar;

// The native window.
class NATIVEUI_EXPORT Window : public base::RefCounted<Window> {
 public:
  struct Options {
    RectF bounds;
  };

  explicit Window(const Options& options);

  void Close();

  void SetContentView(Container* view);
  Container* GetContentView() const;

  void SetContentBounds(const RectF& bounds);
  RectF GetContentBounds() const;

  void SetBounds(const RectF& bounds);
  RectF GetBounds() const;

  void SetVisible(bool visible);
  bool IsVisible() const;

#if defined(OS_WIN) || defined(OS_LINUX)
  void SetMenuBar(MenuBar* menu_bar);
  MenuBar* GetMenuBar() const;
#endif

  void SetBackgroundColor(Color color);

  // Events.
  Signal<void()> on_close;

  // Delegate methods.
  base::Callback<bool()> should_close;

  // Internal: The native window object.
  NativeWindow window() const { return window_; }

 protected:
  virtual ~Window();

 private:
  friend class base::RefCounted<Window>;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit(const Options& options);
  void PlatformDestroy();
  void PlatformSetContentView(Container* container);
#if defined(OS_WIN) || defined(OS_LINUX)
  void PlatformSetMenuBar(MenuBar* menu_bar);
#endif

#if defined(OS_WIN) || defined(OS_LINUX)
  scoped_refptr<MenuBar> menu_bar_;
#endif

  NativeWindow window_;
  scoped_refptr<Container> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_WINDOW_H_
