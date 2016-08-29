// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/gfx/mac/coordinate_conversion.h"

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

- (void)windowWillClose:(NSNotification*)notification {
  shell_->on_close.Emit();
}

@end

namespace nu {

namespace {

// Converting between window and content bounds.
Rect ContentToWindowBounds(NSWindow* window, const Rect& bounds) {
  Rect window_bounds([window frameRectForContentRect:bounds.ToCGRect()]);
  int frame_height = window_bounds.height() - bounds.height();
  window_bounds.set_y(window_bounds.y() - frame_height);
  return window_bounds;
}

Rect WindowToContentBounds(NSWindow* window, const Rect& bounds) {
  Rect content_bounds([window contentRectForFrameRect:bounds.ToCGRect()]);
  int frame_height = bounds.height() - content_bounds.height();
  content_bounds.set_y(content_bounds.y() + frame_height);
  return content_bounds;
}

}  // namespace

Window::~Window() {
  // Clear the delegate class.
  [[window_ delegate] release];
  [window_ setDelegate:nil];

  [window_ release];
}

void Window::PlatformInit(const Options& options) {
  NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask |
                         NSClosableWindowMask | NSResizableWindowMask |
                         NSTexturedBackgroundWindowMask;
  window_ = [[NSWindow alloc]
      initWithContentRect:ScreenRectToNSRect(options.bounds)
                styleMask:styleMask
                  backing:NSBackingStoreBuffered
                    defer:YES];

  [window_ setDelegate:[[NUWindowDelegate alloc] initWithShell:this]];
  [window_ setReleasedWhenClosed:NO];
}

void Window::Close() {
  [window_ performClose:nil];
}

void Window::PlatformSetContentView(Container* container) {
  [window_ setContentView:container->view()];
  container->Layout();
}

void Window::SetContentBounds(const Rect& bounds) {
  SetBounds(ContentToWindowBounds(window_, bounds));
}

Rect Window::GetContentBounds() const {
  return WindowToContentBounds(window_, GetBounds());
}

void Window::SetBounds(const Rect& bounds) {
  [window_ setFrame:ScreenRectToNSRect(bounds) display:YES animate:NO];
}

Rect Window::GetBounds() const {
  return ScreenRectFromNSRect(NSRectToCGRect([window_ frame]));
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

}  // namespace nu
