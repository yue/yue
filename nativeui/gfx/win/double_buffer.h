// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_DOUBLE_BUFFER_H_
#define NATIVEUI_GFX_WIN_DOUBLE_BUFFER_H_

#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "base/win/scoped_select_object.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

// Create a memory buffer for |dc| and copy the result back in destructor.
class DoubleBuffer {
 public:
  DoubleBuffer(HDC dc, const Size& size, const Rect& src, const Point& dest)
      : dc_(dc), src_(src), dest_(dest),
        mem_dc_(::CreateCompatibleDC(dc)),
        mem_bitmap_(::CreateCompatibleBitmap(dc, size.width(), size.height())),
        select_bitmap_(mem_dc_.Get(), mem_bitmap_.get()) {
  }

  ~DoubleBuffer() {
    // Transfer the off-screen DC to the screen.
    BitBlt(dc_, src_.x(), src_.y(), src_.width(), src_.height(),
           dc(), dest_.x(), dest_.y(), SRCCOPY);
  }

  HDC dc() const { return mem_dc_.Get(); }

 private:
  HDC dc_;
  Rect src_;
  Point dest_;
  base::win::ScopedCreateDC mem_dc_;
  base::win::ScopedBitmap mem_bitmap_;
  base::win::ScopedSelectObject select_bitmap_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_DOUBLE_BUFFER_H_
