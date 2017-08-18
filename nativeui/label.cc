// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

namespace nu {

// static
const char Label::kClassName[] = "Label";

const char* Label::GetClassName() const {
  return kClassName;
}

void Label::SetText(const std::string& text) {
  PlatformSetText(text);
  UpdateDefaultStyle();
}

}  // namespace nu
