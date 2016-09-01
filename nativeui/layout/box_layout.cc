// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/layout/box_layout.h"

namespace nu {

BoxLayout::BoxLayout(Orientation orientation,
                     Insets inner_padding,
                     int child_spacing)
    : orientation_(orientation),
      inner_padding_(inner_padding),
      child_spacing_(child_spacing) {
}

BoxLayout::~BoxLayout() {
}

void BoxLayout::Layout(Container* host) {
  if (host->child_count() == 0)
    return;

  Rect bounds(host->GetPixelBounds().size());
  if (bounds.IsEmpty())
    return;

  // Calculate from where to place the views.
  Size preferred_size(host->preferred_size());
  bounds.Inset(inner_padding_);
  Point origin(bounds.x() + (bounds.width() - preferred_size.width()) / 2,
               bounds.y() + (bounds.height() - preferred_size.height()) / 2);

  if (host->child_count() == 1) {
    // No spacing for single child.
    host->child_at(0)->SetPixelBounds(Rect(origin, preferred_size));
    return;
  }

  for (int i = 0; i < host->child_count(); ++i) {
    View* child = host->child_at(i);
    child->SetPixelBounds(Rect(origin, child->preferred_size()));
    if (orientation_ == Horizontal)
      origin.Offset(child->preferred_size().width() + child_spacing_, 0);
    else
      origin.Offset(0, child->preferred_size().height() + child_spacing_);
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
