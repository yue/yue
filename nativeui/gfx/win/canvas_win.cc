// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/canvas.h"

#include "nativeui/gfx/win/painter_win.h"

namespace nu {

namespace {

const float kDefaultDPI = 96.f;

}  // namespace

// static
NativeBitmap Canvas::PlatformCreateBitmap(const SizeF& size,
                                          float scale_factor) {
  NativeBitmap bitmap = new Gdiplus::Bitmap(size.width() * scale_factor,
                                            size.height() * scale_factor,
                                            PixelFormat32bppARGB);
  float dpi = kDefaultDPI * scale_factor;
  bitmap->SetResolution(dpi, dpi);
  return bitmap;
}

// static
void Canvas::PlatformDestroyBitmap(NativeBitmap bitmap) {
  delete bitmap;
}

// static
Painter* Canvas::PlatformCreatePainter(NativeBitmap bitmap,
                                       float scale_factor) {
  return new PainterWin(bitmap, scale_factor);
}

}  // namespace nu
