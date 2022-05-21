// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/separator.h"

#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUSeparator : NSBox<NUViewMethods> {
 @private
  nu::Orientation orientation_;
  nu::NUViewPrivate private_;
}
- (id)initWithShell:(nu::Separator*)shell
        orientation:(nu::Orientation)orientation;
- (nu::Orientation)orientation;
@end

@implementation NUSeparator

- (id)initWithShell:(nu::Separator*)shell
        orientation:(nu::Orientation)orientation {
  if ((self = [super init])) {
    [self setBoxType:NSBoxSeparator];
    orientation_ = orientation;
  }
  return self;
}

- (nu::Orientation)orientation {
  return orientation_;
}

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
  return false;
}

@end

namespace nu {

NativeView Separator::PlatformCreate(Orientation orientation) {
  return [[NUSeparator alloc] initWithShell:this
                                orientation:orientation];
}

}  // namespace nu
