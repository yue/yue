// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_BOX_LAYOUT_H_
#define NATIVEUI_LAYOUT_BOX_LAYOUT_H_

#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/layout/layout_manager.h"

namespace nu {

NATIVEUI_EXPORT class BoxLayout : public LayoutManager {
 public:
  enum Orientation {
    Horizontal,
    Vertical,
  };
  BoxLayout(Orientation orientation,
            Insets inner_padding = Insets(),
            int child_spacing = 0);

  // LayoutManager:
  void Layout(Container* host) override;
  Size GetPreferredSize(Container* host) override;

 protected:
  ~BoxLayout() override;

 private:
  Orientation orientation_;

  // The padding between the broder.
  Insets inner_padding_;

  // The space between children.
  int child_spacing_;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_BOX_LAYOUT_H_
