// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_STATE_H_
#define NATIVEUI_STATE_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"

#if defined(OS_LINUX)
#include "nativeui/gtk/gtk_event_loop.h"
#elif defined(OS_WIN)
#include "nativeui/win/subwin_holder.h"
#include "nativeui/win/util/class_registrar.h"
#endif

namespace nu {

class PlatformFont;

NATIVEUI_EXPORT class State {
 public:
  State();
  ~State();

  static State* current();

  // Returns the default GUI font.
  PlatformFont* GetDefaultFont();

#if defined(OS_WIN)
  HWND GetSubwinHolder();
  ClassRegistrar* GetClassRegistrar();
#endif

 private:
  void PlatformInit();
  void PlatformDestroy();

  scoped_refptr<PlatformFont> default_font_;

#if defined(OS_LINUX)
  GtkEventLoop gtk_event_loop_;
#endif

#if defined(OS_WIN)
  ULONG_PTR token_;
  std::unique_ptr<SubwinHolder> subwin_holder_;
  std::unique_ptr<ClassRegistrar> class_registrar_;
#endif

  DISALLOW_COPY_AND_ASSIGN(State);
};

}  // namespace nu

#endif  // NATIVEUI_STATE_H_
