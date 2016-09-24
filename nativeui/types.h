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
@class NSFont;
@class NSMenu;
@class NSMenuItem;
@class NSView;
@class NSWindow;
#else
class NSFont;
class NSMenu;
class NSMenuItem;
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
using String = base::string16;
#else
using String = std::string;
#endif

#if defined(OS_MACOSX)
using NativeView = NSView*;
using NativeWindow = NSWindow*;
using NativeFont = NSFont*;
using NativeMenu = NSMenu*;
using NativeMenuItem = NSMenuItem*;
#elif defined(OS_LINUX)
using NativeView = GtkWidget*;
using NativeWindow = GtkWindow*;
using NativeMenu = GtkMenuShell*;
using NativeMenuItem = GtkMenuItem*;
#elif defined(OS_WIN)
using NativeView = BaseView*;
using NativeWindow = WindowImpl*;
using NativeFont = Gdiplus::Font*;
#elif defined(OS_IOS)
using NativeView = UIView*;
using NativeWindow UIWindow*;
using NativeFont = UIFont*;
#endif

}  // namespace nu

#endif  // NATIVEUI_TYPES_H_
