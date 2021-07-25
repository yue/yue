// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/notification.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"
#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/image.h"

namespace nu {

void Notification::Show() {
  [NSUserNotificationCenter.defaultUserNotificationCenter
      deliverNotification:notification_];
}

void Notification::Close() {
  [NSUserNotificationCenter.defaultUserNotificationCenter
      removeDeliveredNotification:notification_];
}

void Notification::SetTitle(const std::string& title) {
  [notification_ setTitle:base::SysUTF8ToNSString(title)];
}

void Notification::SetBody(const std::string& body) {
  [notification_ setInformativeText:base::SysUTF8ToNSString(body)];
}

void Notification::SetInfo(const std::string& info) {
  [notification_ setUserInfo:@{
      @"info": base::SysUTF8ToNSString(info),
      @"actionInfo": [notification_ userInfo][@"actionInfo"],
  }];
}

std::string Notification::GetInfo() const {
  return base::SysNSStringToUTF8([notification_ userInfo][@"info"]);
}

void Notification::SetSilent(bool silent) {
  [notification_ setSoundName:(silent ? nil
                                      : NSUserNotificationDefaultSoundName)];
}

void Notification::SetImagePath(const base::FilePath& path) {
  NSString* imagePath = base::SysUTF8ToNSString(path.value());
  base::scoped_nsobject<NSImage> image(
      [[NSImage alloc] initByReferencingFile:imagePath]);
  [notification_ setContentImage:image.get()];
}

void Notification::SetHasReplyButton(bool has) {
  [notification_ setHasReplyButton:has];
}

void Notification::SetResponsePlaceholder(const std::string& placeholder) {
  [notification_ setResponsePlaceholder:base::SysUTF8ToNSString(placeholder)];
}

void Notification::SetIdentifier(const std::string& identifier) {
  [notification_ setIdentifier:base::SysUTF8ToNSString(identifier)];
}

std::string Notification::GetIdentifier() const {
  return base::SysNSStringToUTF8([notification_ identifier]);
}

void Notification::SetActions(const std::vector<Action>& actions) {
  if (actions.empty()) {
    [notification_ setHasActionButton:NO];
    [notification_ setAdditionalActions:@[]];
    return;
  }

  // When having reply button, the action button does not work and we have to
  // put all actions as additionalActions.
  // Otherwise make the first action as actionButton, and the reset as
  // additionalActions.
  size_t first_as_action = ![notification_ hasReplyButton];
  if (first_as_action) {
    const Action& a = actions[0];
    [notification_ setHasActionButton:YES];
    [notification_ setActionButtonTitle:base::SysUTF8ToNSString(a.title)];
    [notification_ setUserInfo:@{
        @"info": [notification_ userInfo][@"info"],
        @"actionInfo": base::SysUTF8ToNSString(a.info),
    }];
    if (actions.size() == 1) {
      [notification_ setAdditionalActions:@[]];
      return;
    }
  }

  auto* nsactions = [NSMutableArray arrayWithCapacity:actions.size()];
  for (size_t i = first_as_action ? 1 : 0; i < actions.size(); ++i) {
    const auto& action = actions[i];
    auto* nsaction = [NSUserNotificationAction
        actionWithIdentifier:base::SysUTF8ToNSString(action.info)
                       title:base::SysUTF8ToNSString(action.title)];
    [nsactions addObject:nsaction];
  }
  [notification_ setAdditionalActions:nsactions];
}

void Notification::PlatformInit() {
  notification_ = [[NSUserNotification alloc] init];
  [notification_ setHasActionButton:NO];
  [notification_ setUserInfo:@{@"info": @"", @"actionInfo": @""}];

  NSString* identifier =
      [NSString stringWithFormat:@"%@.notification.%@",
                                 [[NSBundle mainBundle] bundleIdentifier],
                                 [[NSUUID UUID] UUIDString]];
  [notification_ setIdentifier:identifier];
}

void Notification::PlatformDestroy() {
  [notification_ release];
}

void Notification::PlatformSetImage(Image* image) {
  [notification_ setContentImage:(image ? image->GetNative() : nil)];
}

}  // namespace nu
