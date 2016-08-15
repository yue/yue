// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include <vector>

#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

// The base class for all kinds of views.
NATIVEUI_EXPORT class View {
 public:
  NativeView view() const { return view_; }

 protected:
  View();
  virtual ~View();

  void set_view(NativeView view) { view_ = view; }

 private:
  NativeView view_;

  DISALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
