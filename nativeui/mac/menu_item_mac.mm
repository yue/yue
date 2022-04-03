// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_item.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/image.h"
#include "nativeui/menu.h"

@interface NUMenuItemDelegate : NSObject {
 @private
  nu::MenuItem* shell_;
}
- (id)initWithShell:(nu::MenuItem*)shell;
- (IBAction)onClick:(id)sender;
@end

@implementation NUMenuItemDelegate

- (id)initWithShell:(nu::MenuItem*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onClick:(id)sender {
  shell_->EmitClick();
}

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item {
  if (!shell_->validate)
    return YES;
  return shell_->validate(shell_);
}

@end

namespace nu {

namespace {

// Mapping roles to selectors.
SEL g_sels_map[] = {
  @selector(copy:),
  @selector(cut:),
  @selector(paste:),
  @selector(selectAll:),
  @selector(undo:),
  @selector(redo:),
  @selector(performMiniaturize:),
  @selector(performZoom:),
  @selector(performClose:),
  @selector(about:),
  @selector(hide:),
  @selector(hideOtherApplications:),
  @selector(unhideAllApplications:),
};

static_assert(
    std::size(g_sels_map) == static_cast<size_t>(MenuItem::Role::ItemCount),
    "g_sels_map should be updated with roles");

}  // namespace

void MenuItem::Click() {
  [NSApp sendAction:menu_item_.action to:menu_item_.target from:menu_item_];
}

void MenuItem::SetLabel(const std::string& label) {
  menu_item_.title = base::SysUTF8ToNSString(label);
  if (menu_item_.submenu)
    menu_item_.submenu.title = menu_item_.title;
}

std::string MenuItem::GetLabel() const {
  return base::SysNSStringToUTF8(menu_item_.title);
}

void MenuItem::SetChecked(bool checked) {
  menu_item_.state = checked ? NSOnState : NSOffState;
  if (checked && type_ == nu::MenuItem::Type::Radio && menu_)
    FlipRadioMenuItems(menu_, this);
}

bool MenuItem::IsChecked() const {
  return menu_item_.state == NSOnState;
}

void MenuItem::SetEnabled(bool enabled) {
  menu_item_.enabled = enabled;
}

bool MenuItem::IsEnabled() const {
  return menu_item_.enabled;
}

void MenuItem::SetVisible(bool visible) {
  menu_item_.hidden = !visible;
}

bool MenuItem::IsVisible() const {
  return !menu_item_.hidden;
}

void MenuItem::PlatformInit() {
  if (type_ == Type::Separator)
    menu_item_ = [[NSMenuItem separatorItem] retain];
  else
    menu_item_ = [[NSMenuItem alloc] init];
  if (role_ < Role::ItemCount) {
    menu_item_.title = @"";  // explicitly set "" to override default title
    menu_item_.target = nil;
    menu_item_.action = g_sels_map[static_cast<int>(role_)];
  } else if (role_ == Role::None) {
    menu_item_.target = [[NUMenuItemDelegate alloc] initWithShell:this];
    menu_item_.action = @selector(onClick:);
  }
}

void MenuItem::PlatformDestroy() {
  [menu_item_.target release];
  [menu_item_ release];
}

void MenuItem::PlatformSetSubmenu(Menu* submenu) {
  menu_item_.submenu = submenu->GetNative();
  menu_item_.submenu.title = menu_item_.title;
  switch (role_) {
    case Role::Help: [NSApp setHelpMenu:menu_item_.submenu]; break;
    case Role::Window: [NSApp setWindowsMenu:menu_item_.submenu]; break;
    case Role::Services: [NSApp setServicesMenu:menu_item_.submenu]; break;
    default: break;
  }
}

void MenuItem::PlatformSetImage(Image* image) {
  menu_item_.image = image->GetNative();
}

}  // namespace nu
