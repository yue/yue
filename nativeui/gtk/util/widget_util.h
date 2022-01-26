// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_UTIL_WIDGET_UTIL_H_
#define NATIVEUI_GTK_UTIL_WIDGET_UTIL_H_

#include <gtk/gtk.h>

#include "base/strings/string_piece.h"
#include "nativeui/gfx/geometry/insets_f.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/types.h"

namespace nu {

class RectF;

bool GtkVersionCheck(int major = 0, int minor = 0, int micro = 0);

SizeF GetPreferredSizeForWidget(NativeView widget);

// Apply CSS |style| on |widget|, the style with same |name| will be
// overwritten.
void ApplyStyle(GtkWidget* widget,
                base::StringPiece name,
                base::StringPiece style);

// Is client-side decoration enabled in window.
bool IsUsingCSD(GtkWindow* window);

// Turn CSD on.
void EnableCSD(GtkWindow* window);

// Turn CSD off and use classic non-decoration.
void DisableCSD(GtkWindow* window);

// Return the insets of native window frame.
// Due to limitations of GTK, this only works for realized window.
bool GetNativeFrameInsets(GtkWidget* window, InsetsF* insets);

// Return the insets of client shadow when using CSD.
// Due to limitations of GTK, this only works for configured window.
bool GetClientShadow(GtkWindow* window,
                     InsetsF* client_shadow,
                     RectF* bounds_without_shadow = nullptr,
                     RectF* bounds_with_shadow = nullptr);

// Force window to allocate size for content view.
void ForceSizeAllocation(GtkWindow* window, GtkWidget* view);

// Resize window's client area ignoring the size request.
void ResizeWindow(GtkWindow* window, bool resizable, int width, int height);

// Create a region from all opaque and semi-transparent points in the context.
cairo_region_t* CreateRegionForNonAlphaArea(cairo_t* cr);

// A helper to call destructor for a type.
template<typename T>
void Delete(void* ptr) {
  delete static_cast<T*>(ptr);
}

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_WIDGET_UTIL_H_
