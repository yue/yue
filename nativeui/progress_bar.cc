// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/progress_bar.h"

namespace nu {

// static
const char ProgressBar::kClassName[] = "ProgressBar";

const char* ProgressBar::GetClassName() const {
  return kClassName;
}

}  // namespace nu
