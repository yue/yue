// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#import <Cocoa/Cocoa.h>

#include "ui/gfx/mac/coordinate_conversion.h"

namespace nu {

Window::~Window() {
  [window_ release];
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

void Window::PlatformInit(const Options& options) {
  NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask |
                         NSClosableWindowMask | NSResizableWindowMask |
                         NSTexturedBackgroundWindowMask;
  window_ = [[NSWindow alloc]
      initWithContentRect:gfx::ScreenRectToNSRect(options.content_bounds)
                styleMask:styleMask
                  backing:NSBackingStoreBuffered
                    defer:YES];
}

void Window::PlatformSetContentView(Container* container) {
  [window_ setContentView:container->view()];
}

}  // namespace nu
