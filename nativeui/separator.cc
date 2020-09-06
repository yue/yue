// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/separator.h"

#include "third_party/yoga/Yoga.h"

namespace nu {

// static
const char Separator::kClassName[] = "Separator";

Separator::Separator(Orientation orientation)
    : orientation_(orientation) {
  TakeOverView(PlatformCreate(orientation));
  UpdateDefaultStyle();

  // Some platforms automatically decides the type of separator depending on
  // the longest axis of the view, in order to have a fixed orientation, we
  // have to make sure the main axis would never be shorter than cross axis.
  if (orientation == Orientation::Horizontal)
    YGNodeStyleSetMaxHeight(node(), kLineHeight);
  else
    YGNodeStyleSetMaxWidth(node(), kLineHeight);
}

Separator::~Separator() {}

const char* Separator::GetClassName() const {
  return kClassName;
}

SizeF Separator::GetMinimumSize() const {
  if (orientation_ == Orientation::Horizontal)
    return SizeF(kLineHeight + 1, kLineHeight);
  else
    return SizeF(kLineHeight, kLineHeight + 1);
}

}  // namespace nu
