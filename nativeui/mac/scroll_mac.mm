// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#import <Cocoa/Cocoa.h>

namespace nu {

void Scroll::PlatformInit(const Size& size) {
  NSScrollView* scroll = [[NSScrollView alloc]
      initWithFrame:Rect(size).ToCGRect()];
  scroll.drawsBackground = NO;
  TakeOverView(scroll);
}

void Scroll::PlatformSetContentView(Container* container) {
  static_cast<NSScrollView*>(view()).documentView = container->view();
}

void Scroll::SetContentSize(const Size& size) {
  GetContentView()->SetBounds(Rect(size));
}

void Scroll::SetScrollBarPolicy(Policy h_policy, Policy v_policy) {
  auto* scroll = static_cast<NSScrollView*>(view());
  scroll.hasHorizontalScroller = (h_policy == Policy::Always) ||
                                 (h_policy == Policy::Automatic);
  scroll.hasVerticalScroller = (v_policy == Policy::Always) ||
                               (v_policy == Policy::Automatic);
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollBarPolicy() const {
  auto* scroll = static_cast<NSScrollView*>(view());
  Policy h_policy = scroll.hasHorizontalScroller ? Policy::Automatic
                                                 : Policy::Never;
  Policy v_policy = scroll.hasVerticalScroller ? Policy::Automatic
                                               : Policy::Never;
  return std::make_tuple(h_policy, v_policy);
}

}  // namespace nu
