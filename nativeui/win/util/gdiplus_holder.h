// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_GDIPLUS_HOLDER_H_
#define NATIVEUI_WIN_UTIL_GDIPLUS_HOLDER_H_

#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

class GdiplusHolder {
 public:
  GdiplusHolder() {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token_, &input, nullptr);
  }

  ~GdiplusHolder() {
    Gdiplus::GdiplusShutdown(token_);
  }

  GdiplusHolder& operator=(const GdiplusHolder&) = delete;
  GdiplusHolder(const GdiplusHolder&) = delete;

 private:
  ULONG_PTR token_;
};


}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_GDIPLUS_HOLDER_H_
