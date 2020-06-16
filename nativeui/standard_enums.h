// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_STANDARD_ENUMS_H_
#define NATIVEUI_STANDARD_ENUMS_H_

namespace nu {

enum class ImageScale {
  None,
  Fill,
  Down,
  UpOrDown,
};

enum class Orientation {
  Horizontal,
  Vertical,
};

}  // namespace nu

#endif  // NATIVEUI_STANDARD_ENUMS_H_
