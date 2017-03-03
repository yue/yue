// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/vibrant.h"

#include "base/mac/sdk_forward_declarations.h"
#include "nativeui/mac/container_mac.h"

@interface NUVibrant : NSVisualEffectView<BaseView> {
 @private
  nu::Vibrant* shell_;
}
- (id)initWithShell:(nu::Vibrant*)shell;
- (nu::View*)shell;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUVibrant

- (id)initWithShell:(nu::Vibrant*)shell {
  self = [super init];
  if (!self)
    return nil;

  shell_ = shell;
  return self;
}

- (nu::View*)shell {
  return shell_;
}

- (void)setNUBackgroundColor:(nu::Color)color {
  [self setNeedsDisplay:YES];
}

@end

namespace nu {

// static
const char Vibrant::kClassName[] = "Vibrant";

Vibrant::Vibrant() : Container("an_empty_constructor") {
  TakeOverView([[NUVibrant alloc] initWithShell:this]);
}

Vibrant::~Vibrant() {
}

const char* Vibrant::GetClassName() const {
  return kClassName;
}

}  // namespace nu
