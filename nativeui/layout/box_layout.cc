// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/layout/box_layout.h"

#include <algorithm>
#include <numeric>

namespace nu {

BoxLayout::BoxLayout(Orientation orientation) : orientation_(orientation) {
}

BoxLayout::~BoxLayout() {
}

void BoxLayout::Layout(Container* host) {
  Rect child_area(host->GetPixelBounds().size());
  if (child_area.IsEmpty())
    return;

  // TODO(zcbenz): Detect hidden children.
  int child_count = host->child_count();
  if (host->child_count() == 0)
    return;

  // The sum of flex value.
  int flex_sum = std::accumulate(
      flex_.begin(),
      flex_.begin() + std::min(static_cast<int>(flex_.size()), child_count),
      0);
  int free_space = 0;

  // Determine the sizes of host area and the preferred size of host.
  Size host_size(host->preferred_size());
  host_size.Enlarge(-inner_padding_.left() - inner_padding_.right(),
                    -inner_padding_.top() - inner_padding_.bottom());
  child_area.Inset(inner_padding_);

  // For stretch main axis alignment, all children are streched to fill the
  // child area of the view.
  int main_axis_size;
  if (flex_sum == 0 && main_axis_alignment_ == Stretch) {
    main_axis_size = orientation_ ==
        Horizontal ? child_area.width() : child_area.height();
    main_axis_size = (main_axis_size - child_spacing_ * (child_count - 1)) /
                     child_count;
  }

  // Calculate the sizes of origins of views.
  Point origin(inner_padding_.left(), inner_padding_.top());
  if (flex_sum == 0) {
    if (orientation_ == Horizontal) {
      if (main_axis_alignment_ == Center)
        origin.Offset((child_area.width() - host_size.width()) / 2, 0);
      else if (main_axis_alignment_ == End)
        origin.Offset(child_area.width() - host_size.width(), 0);
    } else {
      if (main_axis_alignment_ == Center)
        origin.Offset(0, (child_area.height() - host_size.height()) / 2);
      else if (main_axis_alignment_ == End)
        origin.Offset(0, child_area.height() - host_size.height());
    }
  } else {
    // For flex layout, all spaces on main axis will be taken.
    free_space = child_area.width() - host_size.width();
    if (orientation_ == Horizontal)
      host_size.set_width(child_area.width());
    else
      host_size.set_height(child_area.height());
  }

  // Start layout.
  int total_padding = 0;
  int current_flex = 0;
  for (int i = 0; i < host->child_count(); ++i) {
    View* child = host->child_at(i);
    Point child_origin(origin);
    Size child_size(child->preferred_size());
    if (orientation_ == Horizontal) {
      // Decide the size on main axis.
      if (flex_sum > 0) {
        int current_padding = GetPaddingAt(i, free_space, flex_sum,
                                           &total_padding, &current_flex);
        child_size.Enlarge(current_padding, 0);
      } else if (main_axis_alignment_ == Stretch) {
        child_size.set_width(main_axis_size);
      }
      // Decide where to put the view.
      if (cross_axis_alignment_ == Start)
        child_origin.Offset(0, (child_area.height() - host_size.height()) / 2);
      else if (cross_axis_alignment_ == Center)
        child_origin.Offset(0, (child_area.height() - child_size.height()) / 2);
      else if (cross_axis_alignment_ == End)
        child_origin.Offset(0, (child_area.height() - host_size.height()) / 2 +
                               host_size.height() - child_size.height());
      else if (cross_axis_alignment_ == Stretch)
        child_size.set_height(child_area.height());
      // Step to next view.
      origin.Offset(child_size.width() + child_spacing_, 0);
    } else {
      if (flex_sum > 0) {
        int current_padding = GetPaddingAt(i, free_space, flex_sum,
                                           &total_padding, &current_flex);
        child_size.Enlarge(0, current_padding);
      } else if (main_axis_alignment_ == Stretch) {
        child_size.set_height(main_axis_size);
      }
      if (cross_axis_alignment_ == Start)
        child_origin.Offset((child_area.width() - host_size.width()) / 2, 0);
      else if (cross_axis_alignment_ == Center)
        child_origin.Offset((child_area.width() - child_size.width()) / 2, 0);
      else if (cross_axis_alignment_ == End)
        child_origin.Offset((child_area.width() - host_size.width()) / 2 +
                            host_size.width() - child_size.width(), 0);
      else if (cross_axis_alignment_ == Stretch)
        child_size.set_width(child_area.width());
      origin.Offset(0, child_size.height() + child_spacing_);
    }
    child->SetPixelBounds(Rect(child_origin, child_size));
  }
}

Size BoxLayout::GetPreferredSize(Container* host) {
  if (host->child_count() == 0)
    return Size();

  Size size;
  if (host->child_count() == 1) {
    // No spacing when there is only one child.
    size = host->child_at(0)->preferred_size();
  } else {
    if (orientation_ == Horizontal) {
      for (int i = 0; i < host->child_count(); ++i) {
        Size child_size = host->child_at(i)->preferred_size();
        size.set_height(std::max(size.height(), child_size.height()));
        size.set_width(size.width() + child_size.width());
        if (i != host->child_count() - 1)
          size.Enlarge(child_spacing_, 0);
      }
    } else {
      for (int i = 0; i < host->child_count(); ++i) {
        Size child_size = host->child_at(i)->preferred_size();
        size.set_width(std::max(size.width(), child_size.width()));
        size.set_height(size.height() + child_size.height());
        if (i != host->child_count() - 1)
          size.Enlarge(0, child_spacing_);
      }
    }
  }
  size.Enlarge(inner_padding_.left() + inner_padding_.right(),
               inner_padding_.top() + inner_padding_.bottom());
  return size;
}

void BoxLayout::SetFlexAt(int index, int flex) {
  if (static_cast<size_t>(index) >= flex_.size())
    flex_.resize(index + 1, 0);
  flex_[index] = flex;
}

int BoxLayout::GetFlexAt(int index) const {
  return static_cast<size_t>(index) < flex_.size() ?  flex_[index] : 0;
}

int BoxLayout::GetPaddingAt(int index, int free_space, int flex_sum,
                            int* total_padding, int* current_flex) const {
  int flex = GetFlexAt(index);
  if (flex == 0)
    return 0;

  *current_flex += flex;
  int quot = (free_space * *current_flex) / flex_sum;
  int rem = (free_space * *current_flex) % flex_sum;
  int current_padding = quot - *total_padding;
  // Use the current remainder to round to the nearest pixel.
  if (std::abs(rem) * 2 >= flex_sum)
    current_padding += free_space > 0 ? 1 : -1;
  *total_padding += current_padding;
  return current_padding;
}

}  // namespace nu
