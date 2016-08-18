// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WINDOW_H_
#define NATIVEUI_WINDOW_H_

#include "nativeui/view.h"

namespace nu {

// The native window.
NATIVEUI_EXPORT class Window : public base::RefCounted<Window> {
 public:
  struct Options {
    gfx::Rect content_bounds;
  };

  explicit Window(const Options& options);

  void SetContentView(View* view);

  void SetVisible(bool visible);
  bool IsVisible() const;

 protected:
  virtual ~Window();

 private:
  friend class base::RefCounted<Window>;

  NativeWindow window_;
};

}  // namespace nu

#endif  // NATIVEUI_WINDOW_H_
