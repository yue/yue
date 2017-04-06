// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/text.h"

#include "nativeui/state.h"

namespace nu {

TextAttributes::TextAttributes()
    : font(nu::State::GetCurrent()->GetDefaultFont()),
      color(GetSystemColor(SystemColor::Text)),
      align(TextAlign::Start),
      valign(TextAlign::Start) {
}

}  // namespace nu
