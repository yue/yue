// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/vibrant.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

#include "base/mac/sdk_forward_declarations.h"
#include "nativeui/mac/container_mac.h"

@interface NUVibrant : NSVisualEffectView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
}
@end

@implementation NUVibrant

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

- (BOOL)isFlipped {
  return YES;
}

@end

namespace nu {

// static
const char Vibrant::kClassName[] = "Vibrant";

Vibrant::Vibrant() : Container("an_empty_constructor") {
  TakeOverView([[NUVibrant alloc] init]);
}

Vibrant::~Vibrant() {
}

const char* Vibrant::GetClassName() const {
  return kClassName;
}

void Vibrant::SetMaterial(Material material) {
  static_cast<NUVibrant*>(GetNative()).material =
      static_cast<NSVisualEffectMaterial>(material);
}

Vibrant::Material Vibrant::GetMaterial() const {
  return static_cast<Vibrant::Material>(
      static_cast<NUVibrant*>(GetNative()).material);
}

void Vibrant::SetBlendingMode(BlendingMode mode) {
  SetWantsLayer(mode == BlendingMode::WithinWindow);
  static_cast<NUVibrant*>(GetNative()).blendingMode =
      static_cast<NSVisualEffectBlendingMode>(mode);
}

Vibrant::BlendingMode Vibrant::GetBlendingMode() const {
  return static_cast<Vibrant::BlendingMode>(
      static_cast<NUVibrant*>(GetNative()).blendingMode);
}

}  // namespace nu
