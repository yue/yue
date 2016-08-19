// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_
#define NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class LayoutManager : public base::RefCounted<LayoutManager> {
 public:
  virtual void Layout(View* host) = 0;

 protected:
  LayoutManager() {}
  virtual ~LayoutManager() {}

 private:
  friend class base::RefCounted<LayoutManager>;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_
