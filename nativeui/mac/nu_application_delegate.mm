// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_application_delegate.h"

#include "nativeui/lifetime.h"

@implementation NUApplicationDelegate

- (id)initWithShell:(nu::Lifetime*)shell {
  if ((self = [super init]))
    shell_ = shell;
    // Array of file:// urls which may have opened program by open with
    fileURLPaths = [[NSMutableArray alloc]init];
  return self;
}

// Gets sent an event every time a file is opened on mac (file://)
- (void)getURL:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)reply {
  // Convert file handle open request from event to NSString
  NSString* fileURLPath = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
  // Write a recvd file handle open request to an array
  [fileURLPaths addObject:fileURLPath];
}

// Called late enough that all file open events from initial launch can be sent
- (void)applicationWillUpdate:(NSNotification *)notification {
  // Determine amount of fileURLPaths
  NSUInteger totalPaths = [fileURLPaths count];
  // If any fileURLPaths were recvd
  if (totalPaths > 0) {
    // Get last index of fileURLPaths NSMutableArray
    NSUInteger lastIdx = totalPaths - 1;
    // Iterate over file:// open urls while it has path items
    do {
      // Send file:// open url via lifetime.on_open signal
      shell_->on_open.Emit(std::string([fileURLPaths[lastIdx] UTF8String]));
      // Clear file:// open url from the array
      [fileURLPaths removeObjectAtIndex:lastIdx];
      // Decrement last index
      lastIdx--;
    } while (lastIdx != 0);
  }
}

- (void)applicationWillFinishLaunching:(NSNotification*)notify {
  // Ask for an event to be sent to getURL when file is opened with our program
  [[NSAppleEventManager sharedAppleEventManager]
    setEventHandler:self andSelector:@selector(getURL:withReplyEvent:)
    forEventClass:kInternetEventClass andEventID:kAEGetURL];
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
