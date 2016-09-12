// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/layout/box_layout.h"

#include <algorithm>
#include <numeric>

namespace nu {

namespace {

Insets DIPToPixel(View* view, const Insets& insets) {
  return Insets(
      view->DIPToPixel(insets.top()), view->DIPToPixel(insets.left()),
      view->DIPToPixel(insets.bottom()), view->DIPToPixel(insets.right()));
}

}  // namespace

BoxLayout::BoxLayout(Orientation orientation) : orientation_(orientation) {
}

BoxLayout::~BoxLayout() {
}

void BoxLayout::Layout(Container* host) const {
  Rect child_area(host->GetPixelBounds().size());
  if (child_area.IsEmpty())
    return;

  int child_count = 0;
  int flex_sum = 0;
  for (int i = 0; i < host->child_count(); ++i) {
    if (!host->child_at(i)->IsVisible())
      continue;
    child_count++;
    int flex = GetFlexForView(host->child_at(i));
    if (flex == 0)
      continue;
    flex_sum += flex;
  }

  if (child_count == 0)
    return;

  // Convert DIP sizes to pixel size.
  int child_spacing = host->DIPToPixel(child_spacing_);
  Insets inner_padding = DIPToPixel(host, inner_padding_);

  // Determine the sizes of host area and the preferred size of host.
  Size host_size(host->GetPixelPreferredSize());
  host_size.Enlarge(-inner_padding.left() - inner_padding.right(),
                    -inner_padding.top() - inner_padding.bottom());
  child_area.Inset(inner_padding);

  // For stretch main axis alignment, all children are streched to fill the
  // child area of the view.
  int main_axis_size = 0;
  if (flex_sum == 0 && main_axis_alignment_ == Stretch) {
    main_axis_size = orientation_ ==
        Horizontal ? child_area.width() : child_area.height();
    main_axis_size = (main_axis_size - child_spacing * (child_count - 1)) /
                     child_count;
  }

  // Calculate the sizes of origins of views.
  Point origin(inner_padding.left(), inner_padding.top());
  int free_space = 0;
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
    if (orientation_ == Horizontal) {
      free_space = child_area.width() - host_size.width();
      host_size.set_width(child_area.width());
    } else {
      free_space = child_area.height() - host_size.height();
      host_size.set_height(child_area.height());
    }
  }

  // Start layout.
  int total_padding = 0;
  int current_flex = 0;
  for (int i = 0; i < host->child_count(); ++i) {
    View* child = host->child_at(i);
    if (!child->IsVisible())
      continue;
    Point child_origin(origin);
    Size child_size(child->GetPixelPreferredSize());
    if (orientation_ == Horizontal) {
      // Decide the size on main axis.
      if (flex_sum > 0) {
        int current_padding = GetPaddingForView(child, free_space, flex_sum,
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
      origin.Offset(child_size.width() + child_spacing, 0);
    } else {
      if (flex_sum > 0) {
        int current_padding = GetPaddingForView(child, free_space, flex_sum,
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
      origin.Offset(0, child_size.height() + child_spacing);
    }
    child->SetPixelBounds(Rect(child_origin, child_size));
  }
}

Size BoxLayout::GetPixelPreferredSize(Container* host) const {
  if (host->child_count() == 0)
    return Size();

  // Convert DIP sizes to pixel size.
  int child_spacing = host->DIPToPixel(child_spacing_);
  Insets inner_padding = DIPToPixel(host, inner_padding_);

  Size size;
  if (host->child_count() == 1) {
    // No spacing when there is only one child.
    size = host->child_at(0)->GetPixelPreferredSize();
  } else {
    if (orientation_ == Horizontal) {
      for (int i = 0; i < host->child_count(); ++i) {
        if (!host->child_at(i)->IsVisible())
          continue;
        Size child_size = host->child_at(i)->GetPixelPreferredSize();
        size.set_height(std::max(size.height(), child_size.height()));
        size.set_width(size.width() + child_size.width());
        if (i != host->child_count() - 1)
          size.Enlarge(child_spacing, 0);
      }
    } else {
      for (int i = 0; i < host->child_count(); ++i) {
        if (!host->child_at(i)->IsVisible())
          continue;
        Size child_size = host->child_at(i)->GetPixelPreferredSize();
        size.set_width(std::max(size.width(), child_size.width()));
        size.set_height(size.height() + child_size.height());
        if (i != host->child_count() - 1)
          size.Enlarge(0, child_spacing);
      }
    }
  }
  size.Enlarge(inner_padding.left() + inner_padding.right(),
               inner_padding.top() + inner_padding.bottom());
  return size;
}

void BoxLayout::ViewRemoved(View* host, View* view) {
  ClearFlexForView(view);
}

void BoxLayout::SetFlexForView(const View* view, int flex) {
  flex_map_[view] = flex;
}

void BoxLayout::ClearFlexForView(const View* view) {
  DCHECK(view);
  flex_map_.erase(view);
}

int BoxLayout::GetFlexForView(const View* view) const {
  std::map<const View*, int>::const_iterator it = flex_map_.find(view);
  if (it == flex_map_.end())
    return 0;
  return it->second;
}

int BoxLayout::GetPaddingForView(const View* view, int free_space, int flex_sum,
                                 int* total_padding, int* current_flex) const {
  int flex = GetFlexForView(view);
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
