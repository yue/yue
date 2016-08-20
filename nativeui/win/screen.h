// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCREEN_H_
#define NATIVEUI_WIN_SCREEN_H_

#include <windows.h>

#include "ui/gfx/geometry/rect.h"

namespace nu {

// Converts a screen physical rect to a screen DIP rect.
// The DPI scale is performed relative to the display nearest to |hwnd|.
// If |hwnd| is null, scaling will be performed to the display nearest to
// |pixel_bounds|.
gfx::Rect ScreenToDIPRect(HWND hwnd, const gfx::Rect& pixel_bounds);

// Converts a screen DIP rect to a screen physical rect.
// The DPI scale is performed relative to the display nearest to |hwnd|.
// If |hwnd| is null, scaling will be performed to the display nearest to
// |dip_bounds|.
gfx::Rect DIPToScreenRect(HWND hwnd, const gfx::Rect& dip_bounds);

}  // namespace nu

#endif  // NATIVEUI_WIN_SCREEN_H_
