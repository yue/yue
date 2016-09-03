// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_BOX_LAYOUT_H_
#define NATIVEUI_LAYOUT_BOX_LAYOUT_H_

#include <vector>

#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/layout/layout_manager.h"

namespace nu {

NATIVEUI_EXPORT class BoxLayout : public LayoutManager {
 public:
  enum Orientation {
    Horizontal,
    Vertical,
  };
  enum AxisAlignment {
    Start,
    Center,
    End,
    Stretch,
  };
  explicit BoxLayout(Orientation orientation);

  // LayoutManager:
  void Layout(Container* host) override;
  Size GetPreferredSize(Container* host) override;

  // Sets the flex for nth child. Using the preferred size as the basis, free
  // space along the main axis is distributed to views in the ratio of their
  // flex weights. Similarly, if the views will overflow the parent, space is
  // subtracted in these ratios.
  //
  // A flex of 0 means this view is not resized. Flex values must not be
  // negative.
  void SetFlexAt(int flex, int index);

  void set_orientation(Orientation orientation) { orientation_ = orientation; }
  void set_inner_padding(const Insets& padding) { inner_padding_ = padding; }
  void set_child_spacing(int spacing) { child_spacing_ = spacing; }
  void set_main_axis_alignment(AxisAlignment a) { main_axis_alignment_ = a; }
  void set_cross_axis_alignment(AxisAlignment a) { cross_axis_alignment_ = a; }

 protected:
  ~BoxLayout() override;

 private:
  int GetFlexAt(int index) const;

  // Calculate the padding according to the element's flex weight.
  int GetPaddingAt(int index, int free_space, int flex_sum,
                   int* total_padding, int* current_flex) const;

  Orientation orientation_;

  // The padding between the broder.
  Insets inner_padding_;

  // The space between children.
  int child_spacing_ = 0;

  // How children are laid out on the main axis.
  // e.g. a horizontal layout of End will result in the child views being
  // right-aligned.
  AxisAlignment main_axis_alignment_ = Start;

  // How children are laid out on the cross axis.
  // e.g. a horizontal layout of End will result in the child views being
  // bottom-aligned.
  AxisAlignment cross_axis_alignment_ = Stretch;

  // Records the flex for children.
  std::vector<int> flex_;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_BOX_LAYOUT_H_
