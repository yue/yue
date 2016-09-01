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
  if (host->child_count() == 0)
    return;

  if (orientation_ == Horizontal) {
    int per_width = host->GetPixelBounds().width() / host->child_count();
    int height = host->GetPixelBounds().height();
    for (int i = 0; i < host->child_count(); ++i) {
      host->child_at(i)->SetPixelBounds(
          Rect(per_width * i, 0, per_width, height));
    }
  } else {
    int per_height = host->GetPixelBounds().height() / host->child_count();
    int width = host->GetPixelBounds().width();
    for (int i = 0; i < host->child_count(); ++i) {
      host->child_at(i)->SetPixelBounds(
          Rect(0, per_height * i, width, per_height));
    }
  }
}

Size BoxLayout::GetPreferredSize(Container* host) {
  if (host->child_count() == 0)
    return Size();

  Size size;
  if (orientation_ == Horizontal) {
    for (int i = 0; i < host->child_count(); ++i) {
      Size child_size = host->child_at(i)->preferred_size();
      size.set_height(std::max(size.height(), child_size.height()));
      size.set_width(size.width() + child_size.width());
    }
  } else {
    for (int i = 0; i < host->child_count(); ++i) {
      Size child_size = host->child_at(i)->preferred_size();
      size.set_width(std::max(size.width(), child_size.width()));
      size.set_height(size.height() + child_size.height());
    }
  }
  return size;
}

}  // namespace nu
