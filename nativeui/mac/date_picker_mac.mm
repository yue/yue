// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/date_picker.h"

#import <Cocoa/Cocoa.h>

#include "base/time/time.h"
#include "nativeui/gfx/font.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUDatePicker : NSDatePicker<NUView> {
 @private
  nu::DatePicker* shell_;
  nu::NUPrivate private_;
}
- (id)initWithShell:(nu::DatePicker*)shell;
- (void)onDateChange:(id)sender;
@end

@implementation NUDatePicker

- (id)initWithShell:(nu::DatePicker*)shell {
  if ((self = [super init])) {
    shell_ = shell;
    [self setTarget:self];
    [self setAction:@selector(onDateChange:)];
    [self setDateValue:[NSDate date]];
  }
  return self;
}

- (void)onDateChange:(id)sender {
  shell_->on_date_change.Emit(shell_);
}

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  [self setFont:font->GetNative()];
}

- (void)setNUColor:(nu::Color)color {
  [self setTextColor:color.ToNSColor()];
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [self setBackgroundColor:color.ToNSColor()];
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

@end

namespace nu {

DatePicker::DatePicker(const Options& options) {
  NSDatePicker* picker = [[NUDatePicker alloc] initWithShell:this];
  [picker setDatePickerElements:options.elements];
  [picker setDatePickerStyle:options.has_stepper ?
      NSDatePickerStyleTextFieldAndStepper : NSDatePickerStyleTextField];
  TakeOverView(picker);
  UpdateDefaultStyle();
}

void DatePicker::SetDate(const base::Time& time) {
  NSDate* date = time.ToNSDate();
  [static_cast<NSDatePicker*>(GetNative()) setDateValue:date];
}

base::Time DatePicker::GetDate() const {
  NSDate* date = [static_cast<NSDatePicker*>(GetNative()) dateValue];
  return base::Time::FromNSDate(date);
}

void DatePicker::SetRange(const absl::optional<base::Time>& min,
                          const absl::optional<base::Time>& max) {
  auto* picker = static_cast<NSDatePicker*>(GetNative());
  [picker setMinDate:min ? min->ToNSDate() : nil];
  [picker setMaxDate:max ? max->ToNSDate() : nil];
}

std::tuple<absl::optional<base::Time>,
           absl::optional<base::Time>> DatePicker::GetRange() const {
  auto* picker = static_cast<NSDatePicker*>(GetNative());
  return std::make_tuple(
      [picker minDate] ? base::Time::FromNSDate([picker minDate])
                       : absl::optional<base::Time>(),
      [picker maxDate] ? base::Time::FromNSDate([picker maxDate])
                       : absl::optional<base::Time>());
}

bool DatePicker::HasStepper() const {
  auto* picker = static_cast<NSDatePicker*>(GetNative());
  return [picker datePickerStyle] == NSDatePickerStyleTextFieldAndStepper;
}

SizeF DatePicker::GetMinimumSize() const {
  return SizeF(static_cast<NSControl*>(GetNative()).intrinsicContentSize);
}

}  // namespace nu
