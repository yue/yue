// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/mac_util.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_responder.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/mac/nu_window.h"
#include "third_party/yoga/Yoga.h"

#if defined(OS_MAC)
#include "nativeui/toolbar.h"
#endif

@interface NUWindow : NSWindow<NUWindowMethods> {
 @private
  nu::NUWindowPrivate private_;
}
@end

@implementation NUWindow

- (nu::NUWindowPrivate*)nuPrivate {
  return &private_;
}

@end

@interface NUWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  nu::Window* shell_;
}
- (id)initWithShell:(nu::Window*)shell;
@end

@implementation NUWindowDelegate

- (id)initWithShell:(nu::Window*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (BOOL)windowShouldClose:(id)sender {
  return !shell_->should_close || shell_->should_close(shell_);
}

- (void)windowWillClose:(NSNotification*)notification {
  shell_->NotifyWindowClosed();
}

- (void)windowDidBecomeMain:(NSNotification*)notification {
  shell_->on_focus.Emit(shell_);
}

- (void)windowDidResignMain:(NSNotification*)notification {
  shell_->on_blur.Emit(shell_);
}

- (void)windowDidResize:(NSNotification*)notification {
  // NSWindow does not have a updateTrackingAreas method, so update on resize.
  [shell_->GetNative() updateTrackingAreas];
}

@end

namespace nu {

// static
Window* Window::FromNative(NativeWindow window) {
  if (!IsNUResponder(window))
    return nullptr;
  return [window shell];
}

void Window::PlatformInit(const Options& options) {
  InstallNUWindowMethods([NUWindow class]);

  NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask |
                         NSClosableWindowMask | NSResizableWindowMask |
                         NSTexturedBackgroundWindowMask;
  NUWindow* window = [[NUWindow alloc]
      initWithContentRect:NSZeroRect
                styleMask:styleMask
                  backing:NSBackingStoreBuffered
                    defer:YES];
  InitResponder(window_ = window, Type::Window);

  [window_ nuPrivate]->shell = this;
  [window_ setDelegate:[[NUWindowDelegate alloc] initWithShell:this]];
  [window_ setReleasedWhenClosed:NO];

  // Disable tab menu items.
  if (base::mac::IsAtLeastOS10_12())
    [window_ setTabbingMode:NSWindowTabbingModeDisallowed];

  YGConfigSetPointScaleFactor(yoga_config_,
                              [window_ screen].backingScaleFactor);

  if (!HasFrame()) {
    // Remove title bar.
    [window_ setTitlebarAppearsTransparent:YES];
    [window_ setTitleVisibility:NSWindowTitleHidden];

    // The fullscreen button should always be hidden for frameless window.
    [[window_ standardWindowButton:NSWindowFullScreenButton] setHidden:YES];

    if (!options.show_traffic_lights) {
      // Hide the window buttons.
      [[window_ standardWindowButton:NSWindowZoomButton] setHidden:YES];
      [[window_ standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
      [[window_ standardWindowButton:NSWindowCloseButton] setHidden:YES];

      // Some third-party macOS utilities check the zoom button's enabled state
      // to determine whether to show custom UI on hover, so we disable it here
      // to prevent them from doing so in a frameless app window.
      [[window_ standardWindowButton:NSWindowZoomButton] setEnabled:NO];
    }
  }

  if (options.transparent) {
    [window_ setOpaque:NO];
    [window_ setHasShadow:NO];  // to follow other platforms
    [window_ setBackgroundColor:[NSColor clearColor]];
  }
}

void Window::PlatformDestroy() {
  // Clear the delegate class.
  [[window_ delegate] release];
  [window_ setDelegate:nil];
  [window_ nuPrivate]->shell = nullptr;

  // The [window_ release] won't close the window, we have to explicitly ask
  // window to close.
  if (!is_closed_)
    [window_ close];

  [window_ release];
}

void Window::Close() {
  [window_ performClose:nil];
}

void Window::SetHasShadow(bool has) {
  [window_ setHasShadow:has];
}

bool Window::HasShadow() const {
  return [window_ hasShadow];
}

void Window::PlatformSetContentView(View* view) {
  bool has_tracking_area = [window_ hasTrackingArea];
  if (content_view_) {
    if (has_tracking_area)
      [window_ disableTracking];
    [content_view_->GetNative() removeFromSuperview];
    if (IsNUResponder(content_view_->GetNative())) {
      NUViewPrivate* priv = [content_view_->GetNative() nuPrivate];
      priv->is_content_view = false;
      // Revert wantsLayer to default.
      [content_view_->GetNative() setWantsLayer:priv->wants_layer];
    } else {
      [content_view_->GetNative() setWantsLayer:NO];
    }
  }

  NSView* content_view = view->GetNative();
  if (IsNUResponder(content_view))
    [content_view nuPrivate]->is_content_view = true;
  [window_ setContentView:content_view];

  if (!HasFrame()) {
    [content_view setFrame:[[[window_ contentView] superview] bounds]];
    // Make sure top corners are rounded:
    [content_view setWantsLayer:!IsTransparent()];
  }
  if (has_tracking_area)
    [window_ enableTracking];
}

void Window::Center() {
  [window_ center];
}

void Window::SetContentSize(const SizeF& size) {
  if (!HasFrame()) {
    NSRect frame = [window_ frame];
    frame.size = size.ToCGSize();
    [window_ setFrame:frame display:YES animate:NO];
    return;
  }

  [window_ setContentSize:size.ToCGSize()];
}

void Window::SetBounds(const RectF& bounds) {
  [window_ setFrame:ScreenRectToNSRect(bounds) display:YES animate:NO];
}

RectF Window::GetBounds() const {
  return ScreenRectFromNSRect([window_ frame]);
}

void Window::SetSizeConstraints(const SizeF& min_size, const SizeF& max_size) {
  [window_ setMinSize:min_size.ToCGSize()];
  if (max_size.IsEmpty())
    [window_ setMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];
  else
    [window_ setMaxSize:max_size.ToCGSize()];
}

std::tuple<SizeF, SizeF> Window::GetSizeConstraints() const {
  SizeF max_size([window_ maxSize]);
  if ((isinf(max_size.width()) && isinf(max_size.height())) ||
      (max_size.width() == FLT_MAX && max_size.height() == FLT_MAX))
    max_size = SizeF();
  return std::make_tuple(SizeF([window_ minSize]), max_size);
}

void Window::SetContentSizeConstraints(const SizeF& min_size,
                                       const SizeF& max_size) {
  if (!HasFrame()) {
    // setContentMinSize always adds height of titlebar.
    SetSizeConstraints(min_size, max_size);
    return;
  }

  [window_ setContentMinSize:min_size.ToCGSize()];
  if (max_size.IsEmpty())
    [window_ setContentMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];
  else
    [window_ setContentMaxSize:max_size.ToCGSize()];
}

std::tuple<SizeF, SizeF> Window::GetContentSizeConstraints() const {
  if (!HasFrame()) {
    // setContentMinSize always adds height of titlebar.
    return GetSizeConstraints();
  }

  SizeF max_size([window_ maxSize]);
  if ((isinf(max_size.width()) && isinf(max_size.height())) ||
      (max_size.width() == FLT_MAX && max_size.height() == FLT_MAX))
    max_size = SizeF();
  return std::make_tuple(SizeF([window_ contentMinSize]), max_size);
}

void Window::Activate() {
  [NSApp activateIgnoringOtherApps:YES];
  [window_ makeKeyAndOrderFront:nil];
}

void Window::Deactivate() {
  [window_ orderBack:nil];
}

bool Window::IsActive() const {
  return [window_ isKeyWindow];
}

void Window::SetVisible(bool visible) {
  if (visible)
    [window_ orderFrontRegardless];
  else
    [window_ orderOut:nil];
}

bool Window::IsVisible() const {
  return [window_ isVisible];
}

void Window::SetAlwaysOnTop(bool top) {
  [window_ setLevel:(top ? NSFloatingWindowLevel : NSNormalWindowLevel)];
}

bool Window::IsAlwaysOnTop() const {
  return [window_ level] != NSNormalWindowLevel;
}

void Window::SetFullscreen(bool fullscreen) {
  if (fullscreen == IsFullscreen())
    return;

  [window_ toggleFullScreen:nil];
}

bool Window::IsFullscreen() const {
  return [window_ styleMask] & NSFullScreenWindowMask;
}

void Window::Maximize() {
  if (!IsMaximized())
    [window_ zoom:nil];
}

void Window::Unmaximize() {
  if (IsMaximized())
    [window_ zoom:nil];
}

bool Window::IsMaximized() const {
  if (IsResizable()) {
    return [window_ isZoomed];
  } else {
    NSRect rectScreen = [[NSScreen mainScreen] visibleFrame];
    NSRect rectWindow = [window_ frame];
    return (rectScreen.origin.x == rectWindow.origin.x &&
            rectScreen.origin.y == rectWindow.origin.y &&
            rectScreen.size.width == rectWindow.size.width &&
            rectScreen.size.height == rectWindow.size.height);
  }
}

void Window::Minimize() {
  [window_ miniaturize:nil];
}

void Window::Restore() {
  [window_ deminiaturize:nil];
}

bool Window::IsMinimized() const {
  return [window_ isMiniaturized];
}

void Window::SetResizable(bool yes) {
  [window_ setWindowStyle:NSResizableWindowMask on:yes];
}

bool Window::IsResizable() const {
  return [window_ styleMask] & NSResizableWindowMask;
}

void Window::SetMaximizable(bool yes) {
  [[window_ standardWindowButton:NSWindowZoomButton] setEnabled:yes];
}

bool Window::IsMaximizable() const {
  return [[window_ standardWindowButton:NSWindowZoomButton] isEnabled];
}

void Window::SetMinimizable(bool minimizable) {
  [window_ setWindowStyle:NSMiniaturizableWindowMask on:minimizable];
}

bool Window::IsMinimizable() const {
  return [window_ styleMask] & NSMiniaturizableWindowMask;
}

void Window::SetMovable(bool movable) {
  [window_ setMovable:movable];
}

bool Window::IsMovable() const {
  return [window_ isMovable];
}

void Window::SetTitle(const std::string& title) {
  [window_ setTitle:base::SysUTF8ToNSString(title)];
}

std::string Window::GetTitle() const {
  return base::SysNSStringToUTF8([window_ title]);
}

void Window::SetBackgroundColor(Color color) {
  [window_ setBackgroundColor:color.ToNSColor()];
}

void Window::SetToolbar(scoped_refptr<Toolbar> toolbar) {
  toolbar_ = std::move(toolbar);
  [window_ setToolbar:toolbar_->GetNative()];
}

void Window::SetTitleVisible(bool visible) {
  window_.titleVisibility = visible ? NSWindowTitleVisible
                                    : NSWindowTitleHidden;
}

bool Window::IsTitleVisible() const {
  return window_.titleVisibility == NSWindowTitleVisible;
}

void Window::SetFullSizeContentView(bool full) {
  [window_ setWindowStyle:NSFullSizeContentViewWindowMask on:full];
}

bool Window::IsFullSizeContentView() const {
  return [window_ styleMask] & NSFullSizeContentViewWindowMask;
}

void Window::PlatformAddChildWindow(Window* child) {
  [window_ addChildWindow:child->GetNative() ordered:NSWindowAbove];
}

void Window::PlatformRemoveChildWindow(Window* child) {
  [window_ removeChildWindow:child->GetNative()];
}

}  // namespace nu
