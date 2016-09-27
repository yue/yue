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
NATIVEUI_EXPORT class Window : public base::RefCounted<Window> {
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

  void SetBackgroundColor(Color color);

  void SetMenuBar(MenuBar* menu_bar);
  MenuBar* GetMenuBar() const;

  // Events.
  Signal<void()> on_close;

  // Delegate methods.
  base::Callback<bool()> should_close;

  NativeWindow window() const { return window_; }

 protected:
  virtual ~Window();

 private:
  friend class base::RefCounted<Window>;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit(const Options& options);
  void PlatformDestroy();
  void PlatformSetContentView(Container* container);
  void PlatformSetMenuBar(MenuBar* menu_bar);

  NativeWindow window_;
  scoped_refptr<MenuBar> menu_bar_;
  scoped_refptr<Container> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_WINDOW_H_
