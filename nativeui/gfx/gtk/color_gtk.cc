// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

namespace nu {

GdkRGBA Color::ToGdkRGBA() const {
  GdkRGBA rgba = { r() / 255., g() / 255., b() / 255., a() / 255. };
  return rgba;
}

}  // namespace nu
