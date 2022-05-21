// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/combo_box.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUComboBox : NSComboBox<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
}
@end

@implementation NUComboBox

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

@end

@interface NUComboBoxDelegate : NSObject<NSComboBoxDelegate> {
 @private
  nu::ComboBox* shell_;
  std::string currentText_;
}
- (id)initWithShell:(nu::ComboBox*)shell;
@end

@implementation NUComboBoxDelegate

- (id)initWithShell:(nu::ComboBox*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (void)controlTextDidChange:(NSNotification*)notification {
  shell_->on_text_change.Emit(shell_);
}

- (void)comboBoxWillDismiss:(NSNotification*)notification {
  // Reading combobox would return old value at this point, we have to read it
  // in next tick to work around it.
  [self performSelector:@selector(onDissmiss) withObject:nil afterDelay:0];
}

- (void)comboBoxWillPopUp:(NSNotification*)notification {
  currentText_ = shell_->GetText();
}

- (void)onDissmiss {
  if (currentText_ != shell_->GetText()) {
    // The controlTextDidChange does not emit when user selects an item.
    shell_->on_text_change.Emit(shell_);
    // The comboBoxSelectionDidChange does not work as expect, and is actually
    // quite useless. We just emulate the event by comparing text.
    shell_->on_selection_change.Emit(shell_);
  }
}

@end

namespace nu {

ComboBox::ComboBox() : Picker([[NUComboBox alloc] init]) {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  [combobox setDelegate:[[NUComboBoxDelegate alloc] initWithShell:this]];
  UpdateDefaultStyle();
}

ComboBox::~ComboBox() {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  [combobox.delegate release];
  [combobox setDelegate:nil];
}

void ComboBox::SetText(const std::string& text) {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  [combobox setStringValue:base::SysUTF8ToNSString(text)];
}

std::string ComboBox::GetText() const {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  return base::SysNSStringToUTF8([combobox stringValue]);
}

void ComboBox::AddItem(const std::string& text) {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  [combobox addItemWithObjectValue:base::SysUTF8ToNSString(text)];
}

void ComboBox::RemoveItemAt(int index) {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  [combobox removeItemAtIndex:index];
}

std::vector<std::string> ComboBox::GetItems() const {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  NSArray<NSString*>* items = [combobox objectValues];
  std::vector<std::string> result;
  result.reserve([items count]);
  for (NSString* item in items)
    result.emplace_back(base::SysNSStringToUTF8(item));
  return result;
}

void ComboBox::SelectItemAt(int index) {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  [combobox selectItemAtIndex:index];
}

std::string ComboBox::GetSelectedItem() const {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  return base::SysNSStringToUTF8([combobox objectValueOfSelectedItem]);
}

int ComboBox::GetSelectedItemIndex() const {
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  return [combobox indexOfSelectedItem];
}

SizeF ComboBox::GetMinimumSize() const {
  // The intrinsicContentSize returns wrong height.
  auto* combobox = static_cast<NUComboBox*>(GetNative());
  return SizeF(0, [[combobox cell] cellSize].height + 2);
}

}  // namespace nu
