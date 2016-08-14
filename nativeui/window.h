// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WINDOW_H_
#define NATIVEUI_WINDOW_H_

#include <memory>

#include "base/callback_list.h"
#include "nativeui/nativeui_export.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

#if defined(OS_MACOSX)
struct WindowImplMac;
#endif

// The native window.
NATIVEUI_EXPORT class Window {
 public:
  struct Options {
    gfx::Rect bounds;
  };

  explicit Window(const Options& options);
  virtual ~Window();

  void SetVisible(bool visible);
  bool IsVisible() const;

 private:
#if defined(OS_MACOSX)
  // Stores Mac specific data.
  std::unique_ptr<WindowImplMac> impl_;
#endif

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace nu

#endif  // NATIVEUI_WINDOW_H_
