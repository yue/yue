// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/canvas.h"

#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/screen.h"

namespace nu {

Canvas::Canvas(const SizeF& size)
    : Canvas(size, nu::GetScaleFactor()) {
}

Canvas::Canvas(const SizeF& size, float scale_factor)
    : scale_factor_(scale_factor),
      size_(size.IsEmpty() ? SizeF(1, 1) : size),  // can't have empty canvas
      bitmap_(PlatformCreateBitmap(size_, scale_factor)),
      painter_(PlatformCreatePainter(bitmap_, size_, scale_factor)) {
}

Canvas::~Canvas() {
  PlatformDestroyBitmap(bitmap_);
}

}  // namespace nu
