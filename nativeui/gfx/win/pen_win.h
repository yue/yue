// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PEN_WIN_H_
#define NATIVEUI_GFX_WIN_PEN_WIN_H_

#include "nativeui/gfx/pen.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

class PenWin : public Pen {
 public:
  PenWin(Color color, float width);

  Gdiplus::Pen* pen() { return &pen_; }

 private:
  ~PenWin() override;

  Gdiplus::Pen pen_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PEN_WIN_H_
