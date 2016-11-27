// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_VIEW_MAC_H_
#define NATIVEUI_MAC_VIEW_MAC_H_

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

@protocol BaseView
- (nu::View*)shell;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

#endif  // NATIVEUI_MAC_VIEW_MAC_H_
