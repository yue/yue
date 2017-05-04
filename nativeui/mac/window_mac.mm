// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/mac_util.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/mac/nu_window.h"
#include "third_party/yoga/yoga/Yoga.h"

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
  return shell_->should_close.is_null() || shell_->should_close.Run();
}

- (void)windowWillClose:(NSNotification*)notification {
  shell_->on_close.Emit();
}

@end

namespace nu {

void Window::PlatformInit(const Options& options) {
  NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask |
                         NSClosableWindowMask | NSResizableWindowMask |
                         NSTexturedBackgroundWindowMask;
  NUWindow* window = [[NUWindow alloc]
      initWithContentRect:ScreenRectToNSRect(options.bounds)
                styleMask:styleMask
                  backing:NSBackingStoreBuffered
                    defer:YES];
  [window setShell:this];
  window_ = window;

  [window_ setDelegate:[[NUWindowDelegate alloc] initWithShell:this]];
  [window_ setReleasedWhenClosed:NO];

  YGConfigSetPointScaleFactor(yoga_config_,
                              [window_ screen].backingScaleFactor);

  if (!options.frame) {
    // The fullscreen button should always be hidden for frameless window.
    [[window_ standardWindowButton:NSWindowFullScreenButton] setHidden:YES];

    // Showing traffic lights for macOS 10.9 requires special work.
    if (options.show_traffic_lights && base::mac::IsOS10_9()) {
      NSButton* b;
      b = [window_ standardWindowButton:NSWindowZoomButton];
      [[b superview] addSubview:b positioned:NSWindowAbove relativeTo:nil];
      b = [window_ standardWindowButton:NSWindowMiniaturizeButton];
      [[b superview] addSubview:b positioned:NSWindowAbove relativeTo:nil];
      b = [window_ standardWindowButton:NSWindowCloseButton];
      [[b superview] addSubview:b positioned:NSWindowAbove relativeTo:nil];
      return;
    }

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
    [window_ setBackgroundColor:[NSColor clearColor]];
  }
}

void Window::PlatformDestroy() {
  // Clear the delegate class.
  [[window_ delegate] release];
  [window_ setDelegate:nil];

  [window_ release];
}

void Window::Close() {
  [window_ performClose:nil];
}

void Window::PlatformSetContentView(View* view) {
  if (content_view_) {
    [content_view_->GetNative() removeFromSuperview];
    [content_view_->GetNative() setWantsLayer:NO];
    [content_view_->GetNative() nuPrivate]->is_content_view = false;
  }

  NSView* content_view = view->GetNative();
  [content_view nuPrivate]->is_content_view = true;
  [window_ setContentView:content_view];

  if (!HasFrame()) {
    [content_view setFrame:[[[window_ contentView] superview] bounds]];
    // Make sure top corners are rounded:
    if (!IsTransparent())
      [content_view setWantsLayer:YES];
  }

  if (view->GetClassName() == Container::kClassName)
    static_cast<Container*>(view)->Layout();
}

void Window::SetContentSize(const SizeF& size) {
  [window_ setContentSize:size.ToCGSize()];
}

void Window::SetBounds(const RectF& bounds) {
  [window_ setFrame:ScreenRectToNSRect(bounds) display:YES animate:NO];
}

RectF Window::GetBounds() const {
  return ScreenRectFromNSRect([window_ frame]);
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

void Window::SetResizable(bool yes) {
  [static_cast<NUWindow*>(window_) setWindowStyle:NSResizableWindowMask on:yes];
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

void Window::SetBackgroundColor(Color color) {
  [window_ setBackgroundColor:color.ToNSColor()];
}

}  // namespace nu
