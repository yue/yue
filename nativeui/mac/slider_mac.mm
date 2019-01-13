// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/slider.h"

#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUSlider : NSSlider<NUView> {
 @private
  nu::NUPrivate private_;
}
@end

@implementation NUSlider

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
  [self setTrackFillColor:color.ToNSColor()];
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

@interface NUSliderDelegate : NSObject {
 @private
  nu::Slider* shell_;
}
- (id)initWithShell:(nu::Slider*)shell;
- (IBAction)onValueChange:(id)sender;
@end

@implementation NUSliderDelegate

- (id)initWithShell:(nu::Slider*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (IBAction)onValueChange:(id)sender {
  shell_->on_value_change.Emit(shell_);

  NSEvent* event = [[NSApplication sharedApplication] currentEvent];
  if (event.type == NSLeftMouseUp)
    shell_->on_sliding_complete.Emit(shell_);
}

@end

namespace nu {

Slider::Slider() {
  auto* slider = [[NUSlider alloc] init];
  [slider setMaxValue:100.];
  [slider setTarget:[[NUSliderDelegate alloc] initWithShell:this]];
  [slider setAction:@selector(onValueChange:)];
  TakeOverView(slider);
  UpdateDefaultStyle();
}

Slider::~Slider() {
  auto* slider = static_cast<NSSlider*>(GetNative());
  [slider.target release];
  [slider setTarget:nil];
}

void Slider::SetValue(float value) {
  auto* slider = static_cast<NSSlider*>(GetNative());
  [slider setFloatValue:value];
}

float Slider::GetValue() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return [slider floatValue];
}

void Slider::SetStep(float step) {
  auto* slider = static_cast<NSSlider*>(GetNative());
  [slider setAltIncrementValue:step];
}

float Slider::GetStep() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return [slider altIncrementValue];
}

void Slider::SetMaximumValue(float max) {
  auto* slider = static_cast<NSSlider*>(GetNative());
  [slider setMaxValue:max];
}

float Slider::GetMaximumValue() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return [slider maxValue];
}

void Slider::SetMinimumValue(float min) {
  auto* slider = static_cast<NSSlider*>(GetNative());
  [slider setMinValue:min];
}

float Slider::GetMinimumValue() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return [slider minValue];
}

SizeF Slider::GetMinimumSize() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return SizeF(0, slider.intrinsicContentSize.height);
}

}  // namespace nu
