// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/layout/box_layout.h"

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

  // Calculate from where to place the views.
  Size host_size(host->preferred_size());
  child_area.Inset(inner_padding_);
  // Start and Stretch alignments start from child area origin.
  Point origin(child_area.x(), child_area.y());
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

  // For stretch main axis alignment, all children are streched to fill the
  // child area of the view.
  int main_axis_size;
  if (main_axis_alignment_ == Stretch) {
    main_axis_size = orientation_ ==
        Horizontal ? child_area.width() : child_area.height();
    main_axis_size = (main_axis_size - child_spacing_ * (child_count - 1)) /
                     child_count;
  }

  // Start layout.
  for (int i = 0; i < host->child_count(); ++i) {
    View* child = host->child_at(i);
    Point child_origin(origin);
    Size child_size(child->preferred_size());
    if (orientation_ == Horizontal) {
      if (main_axis_alignment_ == Stretch)
        child_size.set_width(main_axis_size);
      if (cross_axis_alignment_ == Start)
        child_origin.Offset(0, (child_area.height() - host_size.height()) / 2);
      else if (cross_axis_alignment_ == Center)
        child_origin.Offset(0, (child_area.height() - child_size.height()) / 2);
      else if (cross_axis_alignment_ == End)
        child_origin.Offset(0, (child_area.height() - host_size.height()) / 2 +
                               host_size.height() - child_size.height());
      else if (cross_axis_alignment_ == Stretch)
        child_size.set_height(child_area.height());
      origin.Offset(child_size.width() + child_spacing_, 0);
    } else {
      if (main_axis_alignment_ == Stretch)
        child_size.set_height(main_axis_size);
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

}  // namespace nu
