// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/pen_win.h"

namespace nu {

PenWin::PenWin(Color color, float width)
    : pen_(ToGdi(color), width) {
}

PenWin::~PenWin() {
}

}  // namespace nu
