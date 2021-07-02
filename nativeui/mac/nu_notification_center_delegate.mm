// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_notification_center_delegate.h"

#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/notification.h"

@implementation NUNotificationCenterDelegate

- (void)addNotification:(nu::Notification*)notification {
  notifications_.push_back(notification);
}

- (void)removeNotification:(nu::Notification*)notification {
  notifications_.remove(notification);
}

- (nu::Notification*)findNotificationFrom:(NSUserNotification*)nsnotification {
  std::string identifier = base::SysNSStringToUTF8([nsnotification identifier]);
  for (auto* n : notifications_) {
    if (n->GetIdentifier() == identifier)
      return n;
  }
  return nullptr;
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter*)center
     shouldPresentNotification:(NSUserNotification*)nsnotification {
  return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
        didDeliverNotification:(NSUserNotification*)nsnotification {
  auto* notification = [self findNotificationFrom:nsnotification];
  if (notification)
    notification->on_show.Emit(notification);
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
       didActivateNotification:(NSUserNotification*)nsnotification {
  auto* notification = [self findNotificationFrom:nsnotification];
  if (notification) {
    switch (nsnotification.activationType) {
      case NSUserNotificationActivationTypeNone:
        break;
      case NSUserNotificationActivationTypeContentsClicked:
        notification->on_click.Emit(notification);
        break;
      case NSUserNotificationActivationTypeReplied:
        notification->on_reply.Emit(
            notification,
            base::SysNSStringToUTF8([nsnotification response].string));
        break;
      case NSUserNotificationActivationTypeActionButtonClicked: {
        NSDictionary* userInfo = [nsnotification userInfo];
        notification->on_action.Emit(
            notification,
            base::SysNSStringToUTF8(userInfo[@"actionInfo"]));
        break;
      }
      case NSUserNotificationActivationTypeAdditionalActionClicked: {
        auto action = [nsnotification additionalActivationAction];
        notification->on_action.Emit(
            notification,
            base::SysNSStringToUTF8([action identifier]));
        break;
      }
    }
  }
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
               didDismissAlert:(NSUserNotification*)nsnotification {
  auto* notification = [self findNotificationFrom:nsnotification];
  if (notification)
    notification->on_close.Emit(notification);
}

- (void)userNotificationCenter:(NSUserNotificationCenter*)center
    didRemoveDeliveredNotifications:(NSArray*)nsnotifications {
  for (NSUserNotification* nsnotification in nsnotifications) {
    [self userNotificationCenter:center
                 didDismissAlert:nsnotification];
  }
}

@end
