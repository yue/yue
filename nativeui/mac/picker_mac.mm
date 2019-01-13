// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/picker.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUPicker : NSPopUpButton<NUView> {
 @private
  nu::NUPrivate private_;
}
@end

@implementation NUPicker

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [self.cell setBackgroundColor:color.ToNSColor()];
}

- (void)setNUEnabled:(BOOL)enabled {
  [self setEnabled:enabled];
}

- (BOOL)isNUEnabled {
  return [self isEnabled];
}

@end

@interface NUPickerDelegate : NSObject {
 @private
  nu::Picker* shell_;
}
- (id)initWithShell:(nu::Picker*)shell;
- (IBAction)onChange:(id)sender;
@end

@implementation NUPickerDelegate

- (id)initWithShell:(nu::Picker*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onChange:(id)sender {
  shell_->on_change.Emit(shell_);
}

@end

namespace nu {

Picker::Picker() {
  auto* picker = [[NUPicker alloc] initWithFrame:NSZeroRect pullsDown:NO];
  [picker setTarget:[[NUPickerDelegate alloc] initWithShell:this]];
  [picker setAction:@selector(onChange:)];
  [picker setPreferredEdge:NSMinYEdge];
  [[picker cell] setArrowPosition:NSPopUpArrowAtBottom];

  TakeOverView(picker);
  UpdateDefaultStyle();
}

Picker::~Picker() {
  auto* picker = static_cast<NUPicker*>(GetNative());
  [picker.target release];
  [picker setTarget:nil];
}

void Picker::AddItem(const std::string& text) {
  auto* picker = static_cast<NUPicker*>(GetNative());
  [picker addItemWithTitle:base::SysUTF8ToNSString(text)];
  [picker synchronizeTitleAndSelectedItem];
}

void Picker::RemoveItemAt(int index) {
  auto* picker = static_cast<NUPicker*>(GetNative());
  [picker removeItemAtIndex:index];
}

std::vector<std::string> Picker::GetItems() const {
  auto* picker = static_cast<NUPicker*>(GetNative());
  NSArray<NSString*>* items = [picker itemTitles];
  std::vector<std::string> result;
  result.reserve([items count]);
  for (NSString* item in items)
    result.emplace_back(base::SysNSStringToUTF8(item));
  return result;
}

void Picker::SelectItemAt(int index) {
  auto* picker = static_cast<NUPicker*>(GetNative());
  [picker selectItemAtIndex:index];
  [picker synchronizeTitleAndSelectedItem];
}

std::string Picker::GetSelectedItem() const {
  auto* picker = static_cast<NUPicker*>(GetNative());
  return base::SysNSStringToUTF8([picker titleOfSelectedItem]);
}

int Picker::GetSelectedItemIndex() const {
  auto* picker = static_cast<NUPicker*>(GetNative());
  return [picker indexOfSelectedItem];
}

SizeF Picker::GetMinimumSize() const {
  auto* picker = static_cast<NUPicker*>(GetNative());
  return SizeF(0, picker.intrinsicContentSize.height);
}

}  // namespace nu
