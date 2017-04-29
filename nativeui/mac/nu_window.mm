// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_window.h"

@implementation NUWindow

- (void)setShell:(nu::Window*)shell {
  shell_ = shell;
}

- (nu::Window*)shell {
  return shell_;
}

@end
