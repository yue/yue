// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/picker.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/insets_f.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

namespace {

inline nu::InsetsF GetPopupButtonInsets(NSButton* button) {
  return nu::InsetsF(0, 2, 3, 3);
}

}  // namespace

@interface NUPicker : NSPopUpButton<NUViewMethods> {
 @private
  nu::Picker* shell_;
  nu::NUViewPrivate private_;
}
- (id)initWithShell:(nu::Picker*)shell;
- (IBAction)onSelectionChange:(id)sender;
@end

@implementation NUPicker

- (id)initWithShell:(nu::Picker*)shell {
  if ((self = [super initWithFrame:NSZeroRect pullsDown:NO])) {
    shell_ = shell;
    [self setTarget:self];
    [self setAction:@selector(onSelectionChange:)];
    [self setPreferredEdge:NSMinYEdge];
    [[self cell] setArrowPosition:NSPopUpArrowAtBottom];
  }
  return self;
}

- (IBAction)onSelectionChange:(id)sender {
  shell_->on_selection_change.Emit(shell_);
}

- (nu::NUViewPrivate*)nuPrivate {
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

// Fix wrong coordinates of NSPopUpButton.
- (void)setFrame:(NSRect)frame {
  nu::RectF bounds(frame);
  bounds.Inset(-GetPopupButtonInsets(self));
  [super setFrame:bounds.ToCGRect()];
}

- (NSRect)frame {
  nu::RectF bounds([super frame]);
  bounds.Inset(GetPopupButtonInsets(self));
  return bounds.ToCGRect();
}

@end

namespace nu {

Picker::Picker() : Picker([[NUPicker alloc] initWithShell:this]) {
  // The subclass overrides GetMinimumSize, so we must let subclass call
  // UpdateDefaultStyle itself.
  UpdateDefaultStyle();
}

Picker::Picker(NativeView view) {
  TakeOverView(view);
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

void Picker::Clear() {
  auto* picker = static_cast<NUPicker*>(GetNative());
  [picker removeAllItems];
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
  RectF bounds = RectF(SizeF([[picker cell] cellSize]));
  bounds.Inset(GetPopupButtonInsets(picker));
  return bounds.size();
}

}  // namespace nu
