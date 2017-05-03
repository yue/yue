// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_WIDGET_UTIL_H_
#define NATIVEUI_GTK_WIDGET_UTIL_H_

#include "nativeui/gfx/geometry/size.h"
#include "nativeui/types.h"

typedef struct _cairo_region cairo_region_t;

namespace nu {

Size GetPreferredSizeForWidget(NativeView widget);

// Like gdk_cairo_region_create_from_surface, but also include semi-transparent
// points into the region.
cairo_region_t* CreateRegionFromSurface(cairo_surface_t* surface);

}  // namespace nu

#endif  // NATIVEUI_GTK_WIDGET_UTIL_H_
