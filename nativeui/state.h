// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_STATE_H_
#define NATIVEUI_STATE_H_

#include "nativeui/gfx/font.h"

namespace nu {

NATIVEUI_EXPORT class State {
 public:
  State();
  ~State();

  static State* current();

  // Returns the default GUI font.
  Font GetDefaultFont() const;

 private:
  void PlatformInit();
  void PlatformDestroy();

  Font default_font_;

#if defined(OS_WIN)
  ULONG_PTR token_;
#endif

  DISALLOW_COPY_AND_ASSIGN(State);
};

}  // namespace nu

#endif  // NATIVEUI_STATE_H_
