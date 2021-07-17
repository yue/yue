// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/notification_center_mac.h"

#include "base/strings/sys_string_conversions.h"

@implementation NUNotificationCenterDelegate

- (id)initWithShell:(nu::NotificationCenter*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (std::string)infoForNotification:(NSUserNotification*)nsnotification {
  return base::SysNSStringToUTF8([nsnotification userInfo][@"info"]);
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter*)center
     shouldPresentNotification:(NSUserNotification*)nsnotification {
  return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
        didDeliverNotification:(NSUserNotification*)nsnotification {
  shell_->on_notification_show.Emit([self infoForNotification:nsnotification]);
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
       didActivateNotification:(NSUserNotification*)nsnotification {
  switch (nsnotification.activationType) {
    case NSUserNotificationActivationTypeNone:
      break;
    case NSUserNotificationActivationTypeContentsClicked:
      shell_->on_notification_click.Emit(
          [self infoForNotification:nsnotification]);
      break;
    case NSUserNotificationActivationTypeReplied:
      shell_->on_notification_reply.Emit(
          [self infoForNotification:nsnotification],
          base::SysNSStringToUTF8([nsnotification response].string));
      break;
    case NSUserNotificationActivationTypeActionButtonClicked: {
      NSDictionary* userInfo = [nsnotification userInfo];
      shell_->on_notification_action.Emit(
          base::SysNSStringToUTF8(userInfo[@"actionInfo"]));
      break;
    }
    case NSUserNotificationActivationTypeAdditionalActionClicked: {
      auto action = [nsnotification additionalActivationAction];
      shell_->on_notification_action.Emit(
          base::SysNSStringToUTF8([action identifier]));
      break;
    }
  }
  [NSUserNotificationCenter.defaultUserNotificationCenter
      removeDeliveredNotification:nsnotification];
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
               didDismissAlert:(NSUserNotification*)nsnotification {
  shell_->on_notification_close.Emit([self infoForNotification:nsnotification]);
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
    didRemoveDeliveredNotifications:(NSArray*)nsnotifications {
  for (NSUserNotification* n in nsnotifications)
    shell_->on_notification_close.Emit([self infoForNotification:n]);
}

@end

namespace nu {

void NotificationCenter::Clear() {
  [NSUserNotificationCenter.defaultUserNotificationCenter
      removeAllDeliveredNotifications];
}

void NotificationCenter::PlatformInit() {
  center_ = [[NUNotificationCenterDelegate alloc] initWithShell:this];
  [NSUserNotificationCenter.defaultUserNotificationCenter setDelegate:center_];
}

void NotificationCenter::PlatformDestroy() {
  [NSUserNotificationCenter.defaultUserNotificationCenter setDelegate:nil];
  [center_ release];
}

}  // namespace nu
