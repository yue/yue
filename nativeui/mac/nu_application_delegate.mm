// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_application_delegate.h"

#include "nativeui/lifetime.h"
#include "nativeui/mac/notification_center_mac.h"

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

  // Dispatch the notification to NotificationCenter. This can happen when the
  // app is started by clicking the notification.
  NSObject* obj = [notify userInfo][NSApplicationLaunchUserNotificationKey];
  if (obj && [obj isKindOfClass:[NSUserNotification class]]) {
    auto* notification = static_cast<NSUserNotification*>(obj);
    auto* center = nu::NotificationCenter::GetCurrent()->GetNative();
    if (center) {
      auto* unc = NSUserNotificationCenter.defaultUserNotificationCenter;
      [center userNotificationCenter:unc didActivateNotification:notification];
    }
  }
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender
                    hasVisibleWindows:(BOOL)flag {
  if (flag)
    return YES;
  shell_->on_activate.Emit();
  return NO;
}

- (void)application:(NSApplication*)sender openFiles:(NSArray*)filenames {
  if (!shell_->open_files) {
    [sender replyToOpenOrPrint:NSApplicationDelegateReplyCancel];
    return;
  }

  std::vector<base::FilePath> paths;
  paths.reserve([filenames count]);
  for (NSString* file in filenames)
    paths.push_back(base::FilePath([file fileSystemRepresentation]));
  auto reply = shell_->open_files(std::move(paths));
  [sender replyToOpenOrPrint:static_cast<NSApplicationDelegateReply>(reply)];
}

@end
