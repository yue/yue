// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#import <Cocoa/Cocoa.h>

#include "nativeui/notification_center.h"

@interface NUNotificationCenterDelegate
    : NSObject<NSUserNotificationCenterDelegate> {
 @private
  nu::NotificationCenter* shell_;
}
- (id)initWithShell:(nu::NotificationCenter*)shell;
- (std::string)infoForNotification:(NSUserNotification*)nsnotification;
@end
