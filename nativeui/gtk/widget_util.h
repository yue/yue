// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_WIDGET_UTIL_H_
#define NATIVEUI_GTK_WIDGET_UTIL_H_

#include <gtk/gtk.h>

#include "nativeui/gfx/geometry/insets_f.h"
#include "nativeui/gfx/geometry/size.h"
#include "nativeui/types.h"

typedef struct _cairo_region cairo_region_t;

namespace nu {

Size GetPreferredSizeForWidget(NativeView widget);

// Like gdk_cairo_region_create_from_surface, but also include semi-transparent
// points into the region.
cairo_region_t* CreateRegionFromSurface(cairo_surface_t* surface);

// Is client-side decoration enabled in window.
bool IsUsingCSD(GtkWindow* window);

// Turn CSD on.
void EnableCSD(GtkWindow* window);

// Turn CSD off and use classic non-decoration.
void DisableCSD(GtkWindow* window);

// Return the insets of client shadow when using CSD.
// Due to limitations of GTK, this only works for configured window.
InsetsF GetClientShadow(GtkWindow* window);

// Force window to allocate size for content view.
void ForceSizeAllocation(GtkWindow* window, GtkWidget* view);

// Resize window's client area ignoring the size request.
void ResizeWindow(GtkWindow* window, bool resizable, int width, int height);

}  // namespace nu

#endif  // NATIVEUI_GTK_WIDGET_UTIL_H_
