// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_BOX_LAYOUT_H_
#define NATIVEUI_LAYOUT_BOX_LAYOUT_H_

#include "nativeui/layout/layout_manager.h"

namespace nu {

class BoxLayout : public LayoutManager {
 public:
  enum Orientation {
    Horizontal,
    Vertical,
  };
  BoxLayout(View* host, Orientation orientation);

  // LayoutManager:
  void Layout() override;

 protected:
  ~BoxLayout() override;

 private:
  Orientation orientation_;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_BOX_LAYOUT_H_
