// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_GDIPLUS_H_
#define NATIVEUI_WIN_GDIPLUS_H_

#include <windows.h>

// Work around the compilation problems when using gdiplus.
#include <algorithm>
namespace Gdiplus {
  using std::max;
  using std::min;
}  // namespace Gdiplus

#include <gdiplus.h>  // NOLINT

#endif  // NATIVEUI_WIN_GDIPLUS_H_
