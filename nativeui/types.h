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
typedef struct _AppIndicator AppIndicator;
typedef struct _GdkCursor GdkCursor;
typedef struct _GdkPixbuf GdkPixbuf;
typedef struct _GdkPixbufAnimation GdkPixbufAnimation;
typedef struct _GtkClipboard GtkClipboard;
typedef struct _GtkFileChooser GtkFileChooser;
typedef struct _GtkMenuItem GtkMenuItem;
typedef struct _GtkMenuShell GtkMenuShell;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;
typedef struct _PangoFontDescription PangoFontDescription;
typedef struct _PangoLayout PangoLayout;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;
typedef union _GdkEvent GdkEvent;
#endif

#if defined(OS_MACOSX)
typedef struct CGContext* CGContextRef;
#ifdef __OBJC__
@class NSMutableAttributedString;
@class NSBitmapImageRep;
@class NSCursor;
@class NSEvent;
@class NSFont;
@class NSGraphicsContext;
@class NSImage;
@class NSPasteboard;
@class NSMenu;
@class NSMenuItem;
@class NSSavePanel;
@class NSStatusItem;
@class NSToolbar;
@class NSView;
@class NSWindow;
#else
class NSMutableAttributedString;
class NSBitmapImageRep;
class NSCursor;
class NSEvent;
class NSFont;
class NSGraphicsContext;
class NSImage;
class NSPasteboard;
class NSMenu;
class NSMenuItem;
class NSSavePanel;
class NSStatusItem;
class NSToolbar;
struct NSView;
class NSWindow;
#endif  // __OBJC__
#endif  // defined(OS_MACOSX)

#if defined(OS_WIN)
#ifndef __IDWriteTextLayout_FWD_DEFINED__
#define __IDWriteTextLayout_FWD_DEFINED__
typedef struct IDWriteTextLayout IDWriteTextLayout;
#endif  // __IDWriteTextLayout_FWD_DEFINED__

namespace Gdiplus {
class Font;
class Graphics;
class Image;
}
#endif  // defined(OS_WIN)

namespace nu {

#if defined(OS_WIN)
class ClipboardImpl;
class DoubleBuffer;
class FileDialogImpl;
class TrayImpl;
class ViewImpl;
class WindowImpl;
struct Win32Message;
struct MenuItemData;
#endif

#if defined(OS_MACOSX)
using NativeAttributedText = NSMutableAttributedString*;
using NativeClipboard = NSPasteboard*;
using NativeCursor = NSCursor*;
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
using NativeTray = NSStatusItem*;
#elif defined(OS_LINUX)
using NativeAttributedText = PangoLayout*;
using NativeClipboard = GtkClipboard*;
using NativeCursor = GdkCursor*;
using NativeEvent = GdkEvent*;
using NativeFileDialog = GtkFileChooser*;
using NativeView = GtkWidget*;
using NativeWindow = GtkWindow*;
using NativeBitmap = cairo_surface_t*;
using NativeImage = GdkPixbufAnimation*;
using nativeGraphicsContext = cairo_t*;
using NativeFont = PangoFontDescription*;
using NativeMenu = GtkMenuShell*;
using NativeMenuItem = GtkMenuItem*;
using NativeTray = AppIndicator*;
#elif defined(OS_WIN)
using NativeAttributedText = IDWriteTextLayout*;
using NativeClipboard = ClipboardImpl*;
using NativeCursor = HCURSOR;
using NativeEvent = Win32Message*;
using NativeFileDialog = FileDialogImpl*;
using NativeView = ViewImpl*;
using NativeWindow = WindowImpl*;
using NativeBitmap = DoubleBuffer*;
using NativeFont = Gdiplus::Font*;
using nativeGraphicsContext = Gdiplus::Graphics*;
using NativeImage = Gdiplus::Image*;
using NativeMenu = HMENU;
using NativeMenuItem = MenuItemData*;
using NativeTray = TrayImpl*;
#elif defined(OS_IOS)
using NativeView = UIView*;
using NativeWindow UIWindow*;
using NativeFont = UIFont*;
#endif

}  // namespace nu

#endif  // NATIVEUI_TYPES_H_
