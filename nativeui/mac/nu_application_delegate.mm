// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_application_delegate.h"

#include "nativeui/lifetime.h"

@implementation NUApplicationDelegate

- (id)initWithShell:(nu::Lifetime*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (void)applicationWillFinishLaunching:(NSNotification*)notify {
  // Don't add the "Enter Full Screen" menu item automatically.
  [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSFullScreenMenuItemEverywhere"];
}

- (void)applicationDidFinishLaunching:(NSNotification*)notify {
  shell_->on_ready.Emit();
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender
                    hasVisibleWindows:(BOOL)flag {
  if (flag)
    return YES;
  shell_->on_activate.Emit();
  return NO;
}

@end
