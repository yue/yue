// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_NOTIFICATION_CENTER_DELEGATE_H_
#define NATIVEUI_MAC_NU_NOTIFICATION_CENTER_DELEGATE_H_

#include <list>

#import <Cocoa/Cocoa.h>

namespace nu {
class Notification;
}

@interface NUNotificationCenterDelegate
    : NSObject<NSUserNotificationCenterDelegate> {
  std::list<nu::Notification*> notifications_;
}
- (void)addNotification:(nu::Notification*)notification;
- (void)removeNotification:(nu::Notification*)notification;
- (nu::Notification*)findNotificationFrom:(NSUserNotification*)nsnotification;
@end

#endif  // NATIVEUI_MAC_NU_NOTIFICATION_CENTER_DELEGATE_H_
