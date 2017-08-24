// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/toolbar.h"

#include "nativeui/gfx/image.h"
#include "nativeui/view.h"

namespace nu {

Toolbar::Item::Item() {
}

Toolbar::Item::Item(const Item& item)
    : label(item.label),
      image(item.image),
      view(item.view),
      on_click(item.on_click),
      subitems(item.subitems) {
}

Toolbar::Item::~Item() {
}

}  // namespace nu
