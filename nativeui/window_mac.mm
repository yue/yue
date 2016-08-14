// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"
#include "ui/gfx/mac/coordinate_conversion.h"

namespace nu {

struct WindowImplMac {
  base::scoped_nsobject<NSWindow> window_;
};

Window::Window(const Options& options)
    : impl_(new WindowImplMac()) {
  NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask |
                         NSClosableWindowMask | NSResizableWindowMask |
                         NSTexturedBackgroundWindowMask;
  impl_->window_.reset([[NSWindow alloc]
      initWithContentRect:gfx::ScreenRectToNSRect(options.content_bounds)
                styleMask:styleMask
                  backing:NSBackingStoreBuffered
                    defer:YES]);
}

Window::~Window() {
}

void Window::SetVisible(bool visible) {
  if (visible)
    [impl_->window_ orderFrontRegardless];
  else
    [impl_->window_ orderOut:nil];
}

bool Window::IsVisible() const {
  return [impl_->window_ isVisible];
}

}  // namespace nu
