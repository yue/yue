// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/vibrant.h"

#import <objc/runtime.h>

#include "base/mac/sdk_forward_declarations.h"
#include "nativeui/mac/container_mac.h"

@interface NUVibrant : NSVisualEffectView<NUView> {
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
}

@end

namespace nu {

namespace {

void InitializeVibrantClass() {
  // Only need to be done once.
  static bool initialized = false;
  if (initialized)
    return;

  // Copy methods from NUContainer to NUVibrant.
  initialized = true;
  Class source = [NUContainer class];
  Class target = [NUVibrant class];
  SEL sels[] = { @selector(adjustSubviews),
                 @selector(isFlipped),
                 @selector(resizeSubviewsWithOldSize:) };
  for (SEL sel : sels) {
    Method method = class_getInstanceMethod(source, sel);
    class_addMethod(target,
                    sel,
                    method_getImplementation(method),
                    method_getTypeEncoding(method));
  }
}

}  // namespace

// static
const char Vibrant::kClassName[] = "Vibrant";

Vibrant::Vibrant() : Container("an_empty_constructor") {
  InitializeVibrantClass();
  TakeOverView([[NUVibrant alloc] initWithShell:this]);
}

Vibrant::~Vibrant() {
}

const char* Vibrant::GetClassName() const {
  return kClassName;
}

}  // namespace nu
