// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include "nativeui/gfx/image.h"

namespace nu {

// static
const char Button::kClassName[] = "Button";

void Button::SetImage(Image* image) {
  image_ = image;
  PlatformSetImage(image);
  UpdateDefaultStyle();
}

void Button::SetTitle(const std::string& title) {
  PlatformSetTitle(title);
  UpdateDefaultStyle();
}

const char* Button::GetClassName() const {
  return kClassName;
}

}  // namespace nu
