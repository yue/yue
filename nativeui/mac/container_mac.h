// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_CONTAINER_MAC_H_
#define NATIVEUI_MAC_CONTAINER_MAC_H_

#include "nativeui/container.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUContainer : NSView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
  nu::Color background_color_;
}
@end

#endif  // NATIVEUI_MAC_CONTAINER_MAC_H_
