// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/canvas.h"

#include "nativeui/gfx/gtk/painter_gtk.h"

namespace nu {

// static
NativeBitmap Canvas::PlatformCreateBitmap(const SizeF& size,
                                          float scale_factor) {
  cairo_surface_t* surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32,
      size.width() * scale_factor,
      size.height() * scale_factor);
  cairo_surface_set_device_scale(surface, scale_factor, scale_factor);
  return surface;
}

// static
void Canvas::PlatformDestroyBitmap(NativeBitmap bitmap) {
  cairo_surface_destroy(bitmap);
}

// static
Painter* Canvas::PlatformCreatePainter(NativeBitmap bitmap,
                                       const SizeF& size,
                                       float scale_factor) {
  return new PainterGtk(bitmap, size, scale_factor);
}

}  // namespace nu
