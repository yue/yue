// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TYPES_H_
#define NATIVEUI_TYPES_H_

#if defined(OS_WIN)
#include <windows.h>
#endif

#include <string>

#include "build/build_config.h"

#if defined(OS_LINUX)
typedef struct _AppIndicator AppIndicator;
typedef struct _GdkCursor GdkCursor;
typedef struct _GdkMonitor GdkMonitor;
typedef struct _GdkPixbuf GdkPixbuf;
typedef struct _GdkPixbufAnimation GdkPixbufAnimation;
typedef struct _GtkClipboard GtkClipboard;
typedef struct _GtkFileChooser GtkFileChooser;
typedef struct _GtkMenuItem GtkMenuItem;
typedef struct _GtkMenuShell GtkMenuShell;
typedef struct _GtkMessageDialog GtkMessageDialog;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;
typedef struct _GDBusProxy GDBusProxy;
typedef struct _PangoFontDescription PangoFontDescription;
typedef struct _PangoLayout PangoLayout;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;
typedef union _GdkEvent GdkEvent;
#endif

#if defined(OS_MAC)
typedef struct CGContext* CGContextRef;
#ifdef __OBJC__
@class NSMutableAttributedString;
@class NSAlert;
@class NSBitmapImageRep;
@class NSCursor;
@class NSEvent;
@class NSFont;
@class NSGraphicsContext;
@class NSImage;
@class NSPasteboard;
@class NSMenu;
@class NSMenuItem;
@class NSUserNotification;
@class NSSavePanel;
@class NSScreen;
@class NSStatusItem;
@class NSToolbar;
@class NSView;
@class NSResponder;
@class NSWindow;
@class NUNotificationCenterDelegate;
#else
class NSMutableAttributedString;
class NSAlert;
class NSBitmapImageRep;
class NSCursor;
class NSEvent;
class NSFont;
class NSGraphicsContext;
class NSImage;
class NSPasteboard;
class NSMenu;
class NSMenuItem;
class NSUserNotification;
class NSSavePanel;
class NSScreen;
class NSStatusItem;
class NSToolbar;
struct NSView;
class NSResponder;
class NSWindow;
class NUNotificationCenterDelegate;
#endif  // __OBJC__
#endif  // defined(OS_MAC)

#if defined(OS_WIN)
namespace Gdiplus {
class Font;
class Graphics;
class Image;
}
#endif

namespace nu {

#if defined(OS_WIN)
class ClipboardImpl;
class ComServerModule;
class DoubleBuffer;
class FileDialogImpl;
class ResponderImpl;
class TrayImpl;
class ViewImpl;
class WindowImpl;
struct AttributedTextImpl;
struct Win32Message;
struct MenuItemData;
struct MessageBoxImpl;
struct NotificationImpl;
#elif defined(OS_LINUX)
struct NotificationData;
#endif

#if defined(OS_MAC)
using NativeAttributedText = NSMutableAttributedString*;
using NativeClipboard = NSPasteboard*;
using NativeCursor = NSCursor*;
using NativeEvent = NSEvent*;
using NativeFileDialog = NSSavePanel*;
using NativeMessageBox = NSAlert*;
using NativeView = NSView*;
using NativeWindow = NSWindow*;
using NativeBitmap = CGContextRef;
using NativeDisplay = NSScreen*;
using NativeImage = NSImage*;
using nativeGraphicsContext = NSGraphicsContext*;
using NativeFont = NSFont*;
using NativeMenu = NSMenu*;
using NativeMenuItem = NSMenuItem*;
using NativeNotification = NSUserNotification*;
using NativeNotificationCenter = NUNotificationCenterDelegate*;
using NativeResponder = NSResponder*;
using NativeToolbar = NSToolbar*;
using NativeTray = NSStatusItem*;
#elif defined(OS_LINUX)
using NativeAttributedText = PangoLayout*;
using NativeClipboard = GtkClipboard*;
using NativeCursor = GdkCursor*;
using NativeDisplay = GdkMonitor*;
using NativeEvent = GdkEvent*;
using NativeFileDialog = GtkFileChooser*;
using NativeMessageBox = GtkMessageDialog*;
using NativeView = GtkWidget*;
using NativeWindow = GtkWindow*;
using NativeBitmap = cairo_surface_t*;
using NativeImage = GdkPixbufAnimation*;
using nativeGraphicsContext = cairo_t*;
using NativeFont = PangoFontDescription*;
using NativeMenu = GtkMenuShell*;
using NativeMenuItem = GtkMenuItem*;
using NativeNotification = NotificationData*;
using NativeNotificationCenter = GDBusProxy*;
using NativeResponder = GtkWidget*;
using NativeTray = AppIndicator*;
#elif defined(OS_WIN)
using NativeAttributedText = AttributedTextImpl*;
using NativeClipboard = ClipboardImpl*;
using NativeCursor = HCURSOR;
using NativeDisplay = HMONITOR;
using NativeEvent = Win32Message*;
using NativeFileDialog = FileDialogImpl*;
using NativeMessageBox = MessageBoxImpl*;
using NativeView = ViewImpl*;
using NativeWindow = WindowImpl*;
using NativeBitmap = DoubleBuffer*;
using NativeFont = Gdiplus::Font*;
using nativeGraphicsContext = Gdiplus::Graphics*;
using NativeImage = Gdiplus::Image*;
using NativeMenu = HMENU;
using NativeMenuItem = MenuItemData*;
using NativeNotification = NotificationImpl*;
using NativeNotificationCenter = ComServerModule*;
using NativeResponder = ResponderImpl*;
using NativeTray = TrayImpl*;
#endif

}  // namespace nu

#endif  // NATIVEUI_TYPES_H_
