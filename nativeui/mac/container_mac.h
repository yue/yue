// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_CONTAINER_MAC_H_
#define NATIVEUI_MAC_CONTAINER_MAC_H_

#include "nativeui/container.h"
#include "nativeui/mac/view_mac.h"

@interface NUContainer : NSView<BaseView> {
 @private
  nu::Container* shell_;
}
- (id)initWithShell:(nu::Container*)shell;
- (nu::View*)shell;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

#endif  // NATIVEUI_MAC_CONTAINER_MAC_H_
