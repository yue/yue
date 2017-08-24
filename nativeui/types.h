// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TYPES_H_
#define NATIVEUI_TYPES_H_

#if defined(OS_WIN)
#include <windows.h>
#endif

#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/strings/string16.h"
#else
#include <string>
#endif

#if defined(OS_LINUX)
typedef struct _GdkPixbuf GdkPixbuf;
typedef struct _GtkFileChooser GtkFileChooser;
typedef struct _GtkMenuItem GtkMenuItem;
typedef struct _GtkMenuShell GtkMenuShell;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;
typedef struct _PangoFontDescription PangoFontDescription;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;
typedef union _GdkEvent GdkEvent;
#endif

#if defined(OS_MACOSX)
typedef struct CGContext* CGContextRef;
#ifdef __OBJC__
@class NSBitmapImageRep;
@class NSEvent;
@class NSFont;
@class NSGraphicsContext;
@class NSImage;
@class NSMenu;
@class NSMenuItem;
@class NSSavePanel;
@class NSToolbar;
@class NSView;
@class NSWindow;
#else
class NSBitmapImageRep;
class NSEvent;
class NSFont;
class NSGraphicsContext;
class NSImage;
class NSMenu;
class NSMenuItem;
class NSSavePanel;
class NSToolbar;
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
class Bitmap;
class Font;
class Graphics;
class Image;
}
#endif

namespace nu {

#if defined(OS_WIN)
class FileDialogImpl;
class ViewImpl;
class WindowImpl;
struct Win32Message;
struct MenuItemData;
#endif

#if defined(OS_MACOSX)
using NativeEvent = NSEvent*;
using NativeFileDialog = NSSavePanel*;
using NativeView = NSView*;
using NativeWindow = NSWindow*;
using NativeBitmap = CGContextRef;
using NativeImage = NSImage*;
using nativeGraphicsContext = NSGraphicsContext*;
using NativeFont = NSFont*;
using NativeMenu = NSMenu*;
using NativeMenuItem = NSMenuItem*;
using NativeToolbar = NSToolbar*;
#elif defined(OS_LINUX)
using NativeEvent = GdkEvent*;
using NativeFileDialog = GtkFileChooser*;
using NativeView = GtkWidget*;
using NativeWindow = GtkWindow*;
using NativeBitmap = cairo_surface_t*;
using NativeImage = GdkPixbuf*;
using nativeGraphicsContext = cairo_t*;
using NativeFont = PangoFontDescription*;
using NativeMenu = GtkMenuShell*;
using NativeMenuItem = GtkMenuItem*;
#elif defined(OS_WIN)
using NativeEvent = Win32Message*;
using NativeFileDialog = FileDialogImpl*;
using NativeView = ViewImpl*;
using NativeWindow = WindowImpl*;
using NativeBitmap = Gdiplus::Bitmap*;
using NativeFont = Gdiplus::Font*;
using nativeGraphicsContext = Gdiplus::Graphics*;
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
