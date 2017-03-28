// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TYPES_H_
#define NATIVEUI_TYPES_H_

#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/strings/string16.h"
#else
#include <string>
#endif

#if defined(OS_LINUX)
typedef struct _GdkPixbuf GdkPixbuf;
typedef struct _GtkMenuItem GtkMenuItem;
typedef struct _GtkMenuShell GtkMenuShell;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;
typedef struct _PangoFontDescription PangoFontDescription;
#endif

#if defined(OS_MACOSX)
#ifdef __OBJC__
@class NSFont;
@class NSImage;
@class NSMenu;
@class NSMenuItem;
@class NSView;
@class NSWindow;
#else
class NSFont;
class NSImage;
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
class Image;
}
#endif

namespace nu {

#if defined(OS_WIN)
class ViewImpl;
struct MenuItemData;
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
using NativeImage = NSImage*;
using NativeFont = NSFont*;
using NativeMenu = NSMenu*;
using NativeMenuItem = NSMenuItem*;
#elif defined(OS_LINUX)
using NativeView = GtkWidget*;
using NativeWindow = GtkWindow*;
using NativeImage = GdkPixbuf*;
using NativeFont = PangoFontDescription*;
using NativeMenu = GtkMenuShell*;
using NativeMenuItem = GtkMenuItem*;
#elif defined(OS_WIN)
using NativeView = ViewImpl*;
using NativeWindow = WindowImpl*;
using NativeFont = Gdiplus::Font*;
using NativeImage = Gdiplus::Image*;
using NativeMenu = HMENU;
using NativeMenuItem = MenuItemData*;
#elif defined(OS_IOS)
using NativeView = UIView*;
using NativeWindow UIWindow*;
using NativeFont = UIFont*;
#endif

}  // namespace nu

#endif  // NATIVEUI_TYPES_H_
