// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TYPES_H_
#define NATIVEUI_TYPES_H_

#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>  // NOLINT
#elif defined(OS_LINUX)
#include <gtk/gtk.h>  // NOLINT
#endif

#if defined(OS_MACOSX)
#ifdef __OBJC__
@class NSView;
@class NSWindow;
#else
struct NSView;
class NSWindow;
#endif  // __OBJC__
#endif

namespace nu {

#if defined(OS_MACOSX)
typedef NSView* NativeView;
typedef NSWindow* NativeWindow;
#elif defined(OS_LINUX)
typedef GtkWidget* NativeView;
typedef GtkWindow* NativeWindow;
#endif

}  // namespace nu

#endif  // NATIVEUI_TYPES_H_
