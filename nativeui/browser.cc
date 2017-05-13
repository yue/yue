// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

namespace nu {

// static
const char Browser::kClassName[] = "Browser";

const char* Browser::GetClassName() const {
  return kClassName;
}

}  // namespace nu
