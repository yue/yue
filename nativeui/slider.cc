// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/slider.h"

namespace nu {

// static
const char Slider::kClassName[] = "Slider";

Slider::Slider() {
  TakeOverView(PlatformCreate());
  UpdateDefaultStyle();
}

Slider::~Slider() {
}

const char* Slider::GetClassName() const {
  return kClassName;
}

}  // namespace nu
