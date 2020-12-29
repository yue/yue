// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/tray.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/mac/coordinate_conversion.h"
#include "nativeui/menu.h"

// The button delegate to catch click events.
@interface NUStatusButtonDelegate : NSObject {
 @private
  nu::Tray* shell_;
}
- (id)initWithShell:(nu::Tray*)shell;
- (IBAction)onClick:(id)sender;
@end

@implementation NUStatusButtonDelegate

- (id)initWithShell:(nu::Tray*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onClick:(id)sender {
  shell_->on_click.Emit(shell_);
}

@end

namespace nu {

namespace {

void InstallStatusButtonDelegate(Tray* tray) {
  NSStatusBarButton* button = [tray->GetNative() button];
  [button setTarget:[[NUStatusButtonDelegate alloc] initWithShell:tray]];
  [button setAction:@selector(onClick:)];
}

}  // namespace

Tray::Tray(scoped_refptr<Image> icon)
    : tray_([[[NSStatusBar systemStatusBar]
                statusItemWithLength:NSSquareStatusItemLength] retain]) {
  SetImage(std::move(icon));
  InstallStatusButtonDelegate(this);
}

Tray::Tray(const std::string& title)
    : tray_([[[NSStatusBar systemStatusBar]
                statusItemWithLength:NSVariableStatusItemLength] retain]) {
  SetTitle(title);
  InstallStatusButtonDelegate(this);
}

Tray::~Tray() {
  Remove();  // macOS does not remove the icon when obj is destroyed
  NSStatusBarButton* button = [tray_ button];
  [button.target release];
  [tray_ release];
}

void Tray::Remove() {
  [[NSStatusBar systemStatusBar] removeStatusItem:tray_];
}

RectF Tray::GetBounds() const {
  NSStatusBarButton* button = [tray_ button];
  NSRect rect = [button convertRect:[button bounds] toView:nil];
  return ScreenRectFromNSRect([[button window] convertRectToScreen:rect]);
}

void Tray::SetTitle(const std::string& title) {
  [tray_ setTitle:base::SysUTF8ToNSString(title)];
}

void Tray::SetImage(scoped_refptr<Image> icon) {
  [[tray_ button] setImage:icon->GetNative()];
}

void Tray::SetPressedImage(scoped_refptr<Image> icon) {
  [[tray_ button] setAlternateImage:icon->GetNative()];
}

void Tray::SetMenu(scoped_refptr<Menu> menu) {
  [tray_ setMenu:menu->GetNative()];
  menu_ = std::move(menu);
}

}  // namespace nu
