// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TYPES_H_
#define NATIVEUI_TYPES_H_

#include "build/build_config.h"

#if defined(OS_LINUX)
#include <gtk/gtk.h>  // NOLINT
#endif

#if defined(OS_WIN)
#include "base/strings/string16.h"
#else
#include <string>
#endif

#if defined(OS_MACOSX)
#ifdef __OBJC__
@class NSView;
@class NSWindow;
#else
struct NSView;
class NSWindow;
#endif  // __OBJC__
#elif defined(OS_IOS)
#ifdef __OBJC__
@class UIFont;
@class UIView;
@class UIWindow;
#else
class UIFont;
class UIView;
class UIWindow;
#endif  // __OBJC__
#endif

#if defined(OS_WIN)
namespace Gdiplus {
class Font;
}
#endif

namespace nu {

#if defined(OS_WIN)
class BaseView;
class WindowImpl;
#endif

#if defined(OS_WIN)
typedef base::string16 String;
#else
typedef std::string String;
#endif

#if defined(OS_MACOSX)
typedef NSView* NativeView;
typedef NSWindow* NativeWindow;
typedef NSFont* NativeFont;
#elif defined(OS_LINUX)
typedef GtkWidget* NativeView;
typedef GtkWindow* NativeWindow;
#elif defined(OS_WIN)
typedef BaseView* NativeView;
typedef WindowImpl* NativeWindow;
typedef Gdiplus::Font* NativeFont;
#elif defined(OS_IOS)
typedef UIView* NativeView;
typedef UIWindow* NativeWindow;
typedef UIFont* NativeFont;
#endif

}  // namespace nu

#endif  // NATIVEUI_TYPES_H_
