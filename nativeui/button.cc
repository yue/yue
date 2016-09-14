// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

namespace nu {

// static
const char Button::kClassName[] = "Button";

const char* Button::GetClassName() const {
  return kClassName;
}

}  // namespace nu
