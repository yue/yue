// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/canvas.h"

#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/gfx/win/painter_win.h"
#include "nativeui/state.h"
#include "nativeui/win/util/subwin_holder.h"

namespace nu {

// static
NativeBitmap Canvas::PlatformCreateBitmap(const SizeF& size,
                                          float scale_factor) {
  return new DoubleBuffer(State::GetCurrent()->GetSubwinHolder(),
                          ToCeiledSize(ScaleSize(size, scale_factor)));
}

// static
void Canvas::PlatformDestroyBitmap(NativeBitmap bitmap) {
  delete bitmap;
}

// static
Painter* Canvas::PlatformCreatePainter(NativeBitmap bitmap,
                                       const SizeF& size,
                                       float scale_factor) {
  return new PainterWin(bitmap->dc(), bitmap->size(), scale_factor);
}

}  // namespace nu
