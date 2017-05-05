// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2005 Red Hat, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/widget_util.h"

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

// Is client-side decoration enabled in window.
bool IsUsingCSD(GtkWindow* window) {
  return gtk_window_get_titlebar(window) != nullptr;
}

void EnableCSD(GtkWindow* window) {
  // Required for CSD to work.
  gtk_window_set_decorated(window, true);

  // Setting a hidden titlebar to force using CSD rendering.
  gtk_window_set_titlebar(window, gtk_label_new("you should not see me"));

  if (!g_object_get_data(G_OBJECT(window), "css-provider")) {
    // Since we are not using any titlebar, we have to override the border
    // radius of the client decoration to avoid having rounded shadow for the
    // rectange window.
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(
        provider,
        // Using 0 would triger a bug of GTK's shadow rendering code.
        "decoration { border-radius: 0.01px; }", -1, nullptr);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(GTK_WIDGET(window)),
        GTK_STYLE_PROVIDER(provider), G_MAXUINT);
    // Store the provider inside window, we may need to remove it later when
    // user sets a custom titlebar.
    g_object_set_data_full(G_OBJECT(window), "css-provider", provider,
                           g_object_unref);
  }
}

void DisableCSD(GtkWindow* window) {
  gtk_window_set_titlebar(window, nullptr);
  gtk_window_set_decorated(window, false);
}

InsetsF GetClientShadow(GtkWindow* window) {
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window));
  DCHECK(gdkwindow) << "Can only get client shadow from mapped window.";
  // Bounds without client shadow.
  int x, y, width, height;
  gtk_window_get_position(window, &x, &y);
  gtk_window_get_size(window, &width, &height);
  // Bounds with client shadow.
  int sx, sy, swidth, sheight;
  gdk_window_get_geometry(gdkwindow, &sx, &sy, &swidth, &sheight);
  return InsetsF(y - sy,
                 x - sx,
                 (sy + sheight) - (y + height),
                 (sx + swidth) - (x + width));
}

void ForceSizeAllocation(GtkWindow* window, GtkWidget* view) {
  GdkRectangle rect = { 0, 0 };
  gtk_window_get_size(window, &rect.width, &rect.height);
  gtk_widget_size_allocate(view, &rect);
}

void ResizeWindow(GtkWindow* window, bool resizable, int width, int height) {
  // Clear current size requests.
  GtkWidget* vbox = gtk_bin_get_child(GTK_BIN(window));
  gtk_widget_set_size_request(GTK_WIDGET(window), -1, -1);
  gtk_widget_set_size_request(vbox, -1, -1);

  if (resizable || !IsUsingCSD(window)) {
    // gtk_window_resize only works for resizable window.
    if (resizable)
      gtk_window_resize(window, width, height);
    else
      gtk_widget_set_size_request(GTK_WIDGET(window), width, height);
  } else {
    // Setting size request on the window results in weird behavior for
    // unresizable CSD windows, probably related to size of shadows.
    gtk_widget_set_size_request(vbox, width, height);
  }

  // Set default size otherwise GTK may do weird things when setting size
  // request or changing resizable property.
  gtk_window_set_default_size(window, width, height);

  // Notify the content view of the resize.
  ForceSizeAllocation(window, vbox);
}

}  // namespace nu
