// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_SCOPED_SET_MAP_MODE_H_
#define NATIVEUI_GFX_WIN_SCOPED_SET_MAP_MODE_H_

#include <windows.h>

#include "base/logging.h"

namespace nu {

// Helper class for setting and restore the map mode on a DC.
class ScopedSetMapMode {
 public:
  ScopedSetMapMode(HDC hdc, int map_mode)
      : hdc_(hdc),
        old_map_mode_(SetMapMode(hdc, map_mode)) {
    DCHECK(hdc_);
    DCHECK_NE(map_mode, 0);
    DCHECK_NE(old_map_mode_, 0);
  }

  ~ScopedSetMapMode() {
    const int mode = SetMapMode(hdc_, old_map_mode_);
    DCHECK_NE(mode, 0);
  }

  ScopedSetMapMode& operator=(const ScopedSetMapMode&) = delete;
  ScopedSetMapMode(const ScopedSetMapMode&) = delete;

 private:
  HDC hdc_;
  int old_map_mode_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_SCOPED_SET_MAP_MODE_H_
