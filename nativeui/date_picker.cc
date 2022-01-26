// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/date_picker.h"

namespace nu {

// static
const char DatePicker::kClassName[] = "DatePicker";

DatePicker::~DatePicker() = default;

const char* DatePicker::GetClassName() const {
  return kClassName;
}

}  // namespace nu
