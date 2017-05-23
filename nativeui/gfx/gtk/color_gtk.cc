// Copyright 2016 Cheng Zhao. All rights reserved.

#include "nativeui/gfx/color.h"

#include <gdk/gdk.h>

namespace nu {

GdkRGBA Color::ToGdkRGBA() const {
  GdkRGBA rgba = { r() / 255., g() / 255., b() / 255., a() / 255. };
  return rgba;
}

}  // namespace nu
