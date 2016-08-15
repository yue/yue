// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/scoped_types_mac.h"
#include "ui/gfx/mac/coordinate_conversion.h"

namespace nu {

Window::Window(const Options& options) {
  NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask |
                         NSClosableWindowMask | NSResizableWindowMask |
                         NSTexturedBackgroundWindowMask;
  window_.Reset([[NSWindow alloc]
      initWithContentRect:gfx::ScreenRectToNSRect(options.content_bounds)
                styleMask:styleMask
                  backing:NSBackingStoreBuffered
                    defer:YES]);
}

Window::~Window() {
}

void Window::SetContentView(View* view) {
  [GetNativeWindow() setContentView:view->GetNativeView()];
}

void Window::SetVisible(bool visible) {
  if (visible)
    [GetNativeWindow() orderFrontRegardless];
  else
    [GetNativeWindow() orderOut:nil];
}

bool Window::IsVisible() const {
  return [GetNativeWindow() isVisible];
}

}  // namespace nu
