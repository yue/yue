// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#import <Cocoa/Cocoa.h>

namespace nu {

View::View() : view_(nil) {
}

View::~View() {
  [view_ release];
}

void View::PlatformAddChildView(View* view) {
  [view_ addSubview:view->view_];
}

void View::PlatformRemoveChildView(View* view) {
  [view->view_ removeFromSuperview];
}

}  // namespace nu
