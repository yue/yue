// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/util/widget_util.h"

#include <algorithm>

#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect_f.h"

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xatom.h>  // XA_CARDINAL
#endif

namespace nu {

bool GtkVersionCheck(int major, int minor, int micro) {
  static int actual_major = gtk_get_major_version();
  if (actual_major > major)
    return true;
  else if (actual_major < major)
    return false;

  static int actual_minor = gtk_get_minor_version();
  if (actual_minor > minor)
    return true;
  else if (actual_minor < minor)
    return false;

  static int actual_micro = gtk_get_micro_version();
  if (actual_micro >= micro)
    return true;
  else
    return false;
}

SizeF GetPreferredSizeForWidget(GtkWidget* widget) {
  GtkRequisition size;
  gtk_widget_get_preferred_size(widget, nullptr, &size);
  return SizeF(size.width, size.height);
}

void ApplyStyle(GtkWidget* widget,
                base::StringPiece name,
                base::StringPiece style) {
  void* old = g_object_get_data(G_OBJECT(widget), name.data());
  if (old)
    gtk_style_context_remove_provider(
        gtk_widget_get_style_context(widget),
        GTK_STYLE_PROVIDER(old));

  GtkCssProvider* provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(
      provider, style.data(), style.length(), nullptr);
  gtk_style_context_add_provider(
      gtk_widget_get_style_context(widget),
      GTK_STYLE_PROVIDER(provider), G_MAXUINT);
  // Store the provider inside widget.
  g_object_set_data_full(G_OBJECT(widget), name.data(), provider,
                         g_object_unref);
}

bool IsUsingCSD(GtkWindow* window) {
  GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(window));
  return gtk_style_context_has_class(context, "csd") ||
         gtk_style_context_has_class(context, "csd-solid");
}

void EnableCSD(GtkWindow* window) {
  // Required for CSD to work.
  gtk_window_set_decorated(window, true);

  // Setting a hidden titlebar to force using CSD rendering.
  gtk_window_set_titlebar(window, gtk_label_new("you should not see me"));

  // Since we are not using any titlebar, we have to override the border
  // radius of the client decoration to avoid having rounded shadow for the
  // rectange window.
  if (!g_object_get_data(G_OBJECT(window), "rectangle-decoration"))
    ApplyStyle(GTK_WIDGET(window), "rectangle-decoration",
               // Using 0 would triger a bug of GTK's shadow rendering code.
               "decoration { border-radius: 0.01px; }");
}

void DisableCSD(GtkWindow* window) {
  gtk_window_set_titlebar(window, nullptr);
  gtk_window_set_decorated(window, false);
}

bool GetNativeFrameInsets(GtkWidget* window, InsetsF* insets) {
#ifdef GDK_WINDOWING_X11
  GdkWindow* gdkwindow = gtk_widget_get_window(window);
  DCHECK(gdkwindow) << "Can only get native frame from realized window.";

  GdkDisplay* display = gdk_window_get_display(gdkwindow);
  if (!GDK_IS_X11_DISPLAY(display))
    return false;

  Atom type;
  int format;
  unsigned long nitems, bytes_after;  // NOLINT(runtime/int)
  unsigned char* data;
  Status status = XGetWindowProperty(
      GDK_DISPLAY_XDISPLAY(display),
      GDK_WINDOW_XID(gdkwindow),
      gdk_x11_get_xatom_by_name_for_display(display, "_NET_FRAME_EXTENTS"),
      0, 4, false, XA_CARDINAL, &type, &format, &nitems, &bytes_after, &data);
  if (status != Success || !data || nitems != 4)
    return false;

  float s = gtk_widget_get_scale_factor(window);
  long* p = reinterpret_cast<long*>(data);  // NOLINT(runtime/int)
  *insets = InsetsF(p[2] / s, p[0] / s, p[3] / s, p[1] / s);
  if (data)
    XFree(data);
  return true;
#else
  return false;
#endif
}

bool GetClientShadow(GtkWindow* window,
                     InsetsF* client_shadow,
                     RectF* bounds_without_shadow,
                     RectF* bounds_with_shadow) {
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window));
  if (!gdkwindow)
    return false;
  // Bounds without client shadow.
  int x, y, width, height;
  gtk_window_get_position(window, &x, &y);
  gtk_window_get_size(window, &width, &height);
  if (bounds_without_shadow)
    *bounds_without_shadow = RectF(x, y, width, height);
  // Bounds with client shadow.
  int sx, sy, swidth, sheight;
  gdk_window_get_geometry(gdkwindow, &sx, &sy, &swidth, &sheight);
  if (bounds_with_shadow)
    *bounds_with_shadow = RectF(sx, sy, swidth, sheight);
  // Client shadow is their insets.
  *client_shadow =  InsetsF(y - sy, x - sx,
                            (sy + sheight) - (y + height),
                            (sx + swidth) - (x + width));
  return true;
}

void ForceSizeAllocation(GtkWindow* window, GtkWidget* view) {
  // Call get_preferred_width before size allocation, otherwise GTK would print
  // warnings like "How does the code know the size to allocate?".
  gint tmp;
  gtk_widget_get_preferred_width(view, &tmp, nullptr);
  gtk_widget_get_preferred_height(view, &tmp, nullptr);

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

cairo_region_t* CreateRegionForNonAlphaArea(cairo_t* cr) {
  // Calculate the extents of the context.
  GdkRectangle extents;
  double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
  extents.x = std::floor(x1);
  extents.y = std::floor(y1);
  extents.width = std::max(std::ceil(x2) - extents.x, 0.);
  extents.height = std::max(std::ceil(y2) - extents.y, 0.);

  // The entire surface is a region if there is no alpha channel.
  cairo_surface_t* surface = cairo_get_target(cr);
  if (cairo_surface_get_content(surface) == CAIRO_CONTENT_COLOR)
    return cairo_region_create_rectangle(&extents);

  cairo_surface_t* image;
  if (cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE ||
      cairo_image_surface_get_format(surface) != CAIRO_FORMAT_A8) {
    // We work on A8 images to get full alpha channel.
    image = cairo_image_surface_create(CAIRO_FORMAT_A8,
                                       extents.width, extents.height);
    cairo_t* image_cr = cairo_create(image);
    cairo_set_source_surface(image_cr, surface, -extents.x, -extents.y);
    cairo_paint(image_cr);
    cairo_surface_flush(image);
    cairo_destroy(image_cr);
  } else {
    image = cairo_surface_reference(surface);
    cairo_surface_flush(image);
  }

  // Iterate through the image.
  uint8_t* data = cairo_image_surface_get_data(image);
  int stride = cairo_image_surface_get_stride(image);

  cairo_region_t* region = cairo_region_create();
  for (int y = 0; y < extents.height; y++) {
    for (int x = 0; x < extents.width; x++) {
      // Find a row with all transparent pixels.
      int ps = x;
      while (x < extents.width) {
        // Only full-transparent pixels are treated as transparent, this is
        // to match the behavior of macOS and Win32.
        if (data[x] == 0)
          break;
        x++;
      }

      if (x > ps) {
        // Add the row to region.
        GdkRectangle rect = {ps, y, x - ps, 1};
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
