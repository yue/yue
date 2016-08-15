// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include <vector>

#include "nativeui/nativeui_export.h"
#include "nativeui/scoped_types.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

// The base class for all kinds of views.
NATIVEUI_EXPORT class View {
 public:
  NativeView GetNativeView() const { return view_.Get(); }

 protected:
  View();
  virtual ~View();

  ScopedNativeView view_;

 private:
  DISALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
