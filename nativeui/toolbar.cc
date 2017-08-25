// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/toolbar.h"

#include "nativeui/gfx/image.h"
#include "nativeui/view.h"

namespace nu {

// static
const char Toolbar::kFlexibleSpaceItemIdentifier[] = "FlexibleSpaceItem";
const char Toolbar::kSpaceItemIdentifier[] = "SpaceItem";

Toolbar::Item::Item() = default;
Toolbar::Item::Item(Item&&) = default;
Toolbar::Item::~Item() = default;
Toolbar::Item& Toolbar::Item::operator=(Item&&) = default;

}  // namespace nu
