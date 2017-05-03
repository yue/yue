// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2005 Red Hat, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/widget_util.h"

#include <gtk/gtk.h>

#include "base/logging.h"
#include "nativeui/gfx/color.h"

namespace nu {

namespace {

bool CairoSurfaceExtents(cairo_surface_t* surface, GdkRectangle* extents) {
  g_return_val_if_fail(surface != NULL, false);
  g_return_val_if_fail(extents != NULL, false);

  double x1, y1, x2, y2;
  cairo_t* cr = cairo_create(surface);
  cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
  cairo_destroy(cr);

  x1 = floor(x1);
  y1 = floor(y1);
  x2 = ceil(x2);
  y2 = ceil(y2);
  x2 -= x1;
  y2 -= y1;

  if (x1 < G_MININT || x1 > G_MAXINT ||
      y1 < G_MININT || y1 > G_MAXINT ||
      x2 > G_MAXINT || y2 > G_MAXINT) {
    extents->x = extents->y = extents->width = extents->height = 0;
    return false;
  }

  extents->x = x1;
  extents->y = y1;
  extents->width = x2;
  extents->height = y2;
  return true;
}

}  // namespace

Size GetPreferredSizeForWidget(GtkWidget* widget) {
  GtkRequisition size;
  gtk_widget_get_preferred_size(widget, nullptr, &size);
  return Size(size.width, size.height);
}

cairo_region_t* CreateRegionFromSurface(cairo_surface_t* surface) {
  GdkRectangle extents;
  CairoSurfaceExtents(surface, &extents);

  if (cairo_surface_get_content(surface) == CAIRO_CONTENT_COLOR)
    return cairo_region_create_rectangle(&extents);

  cairo_surface_t* image;
  if (cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE ||
      cairo_image_surface_get_format(surface) != CAIRO_FORMAT_A8) {
    // Coerce to an A8 image.
    image = cairo_image_surface_create(CAIRO_FORMAT_A8,
                                       extents.width, extents.height);
    cairo_t* cr = cairo_create(image);
    cairo_set_source_surface(cr, surface, -extents.x, -extents.y);
    cairo_paint(cr);
    cairo_destroy(cr);
  } else {
    image = cairo_surface_reference(surface);
  }

  // Flush the surface to make sure that the rendering is up to date.
  cairo_surface_flush(image);

  uint8_t* data = cairo_image_surface_get_data(image);
  int stride = cairo_image_surface_get_stride(image);

  cairo_region_t* region = cairo_region_create();

  for (int y = 0; y < extents.height; y++) {
    for (int x = 0; x < extents.width; x++) {
      // Search for a continuous range of "non transparent pixels".
      int x0 = x;
      while (x < extents.width) {
        uint8_t a = data[x];
        if (a == 0)  // this pixel is "transparent"
          break;
        x++;
      }

      if (x > x0) {
        // Add the pixels(x0, y) to(x, y+1) as a new rectangle in the region.
        GdkRectangle rect;
        rect.x = x0;
        rect.width = x - x0;
        rect.y = y;
        rect.height = 1;
        cairo_region_union_rectangle(region, &rect);
      }
    }

    data += stride;
  }

  cairo_surface_destroy(image);
  cairo_region_translate(region, extents.x, extents.y);
  return region;
}

}  // namespace nu
