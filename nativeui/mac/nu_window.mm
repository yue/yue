// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2013 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_window.h"

@implementation NUWindow

- (void)setShell:(nu::Window*)shell {
  shell_ = shell;
  can_resize_ = true;
}

- (nu::Window*)shell {
  return shell_;
}

- (void)setWindowStyle:(NSUInteger)style on:(bool)yes {
  bool maximizable = [[self standardWindowButton:NSWindowZoomButton] isEnabled];

  // Changing the styleMask of a frameless windows causes it to change size so
  // we explicitly disable resizing while setting it.
  can_resize_ = false;

  if (yes)
    [self setStyleMask:[self styleMask] | style];
  else
    [self setStyleMask:[self styleMask] & ~style];

  can_resize_ = true;

  // Change style mask will make the zoom button revert to default, probably
  // a bug of Cocoa.
  [[self standardWindowButton:NSWindowZoomButton] setEnabled:maximizable];
}

- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen*)screen {
  if (!can_resize_)
    return [self frame];
  return [super constrainFrameRect:frameRect toScreen:screen];
}

- (void)setFrame:(NSRect)windowFrame display:(BOOL)displayViews {
  // constrainFrameRect is not called on hidden windows so disable adjusting
  // the frame directly when resize is disabled
  if (!can_resize_)
    return;
  [super setFrame:windowFrame display:displayViews];
}

@end
