// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/slider.h"

#include "base/mac/scoped_nsobject.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

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


@interface NUSlider : NSSlider<NUView> {
 @private
  base::scoped_nsobject<NUSliderDelegate> delegate_;
  nu::NUPrivate private_;
}
- (id)initWithShell:(nu::Slider*)shell;
@end

@implementation NUSlider

- (id)initWithShell:(nu::Slider*)shell {
  if ((self = [super init])) {
    delegate_.reset([[NUSliderDelegate alloc] initWithShell:shell]);
    [self setMaxValue:100.];
    [self setTarget:delegate_];
    [self setAction:@selector(onValueChange:)];
  }
  return self;
}

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

namespace nu {

NativeView Slider::PlatformCreate() {
  return [[NUSlider alloc] initWithShell:this];
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

void Slider::SetRange(float min, float max) {
  auto* slider = static_cast<NSSlider*>(GetNative());
  [slider setMinValue:min];
  [slider setMaxValue:max];
}

std::tuple<float, float> Slider::GetRange() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return std::make_tuple([slider minValue], [slider maxValue]);
}

SizeF Slider::GetMinimumSize() const {
  auto* slider = static_cast<NSSlider*>(GetNative());
  return SizeF(0, slider.intrinsicContentSize.height);
}

}  // namespace nu
