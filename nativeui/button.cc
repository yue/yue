// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include <utility>

#include "nativeui/gfx/image.h"

namespace nu {

// static
const char Button::kClassName[] = "Button";

Button::~Button() = default;

void Button::SetImage(scoped_refptr<Image> image) {
  image_ = std::move(image);
  PlatformSetImage(image_.get());
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
