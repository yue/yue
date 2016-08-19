// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/layout/fill_layout.h"

namespace nu {

FillLayout::FillLayout(View* host) : LayoutManager(host) {
}

FillLayout::~FillLayout() {
}

void FillLayout::Layout() {
  if (host()->child_count() > 0)
    host()->child_at(0)->SetBounds(host()->GetBounds());
}

}  // namespace nu
