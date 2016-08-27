// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_STATE_H_
#define NATIVEUI_STATE_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"

namespace nu {

class PlatformFont;

NATIVEUI_EXPORT class State {
 public:
  State();
  ~State();

  static State* current();

  // Returns the default GUI font.
  PlatformFont* GetDefaultFont();

 private:
  void PlatformInit();
  void PlatformDestroy();

  scoped_refptr<PlatformFont> default_font_;

#if defined(OS_WIN)
  ULONG_PTR token_;
#endif

  DISALLOW_COPY_AND_ASSIGN(State);
};

}  // namespace nu

#endif  // NATIVEUI_STATE_H_
