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

void Scroll::SetVerticalScrollBar(bool has) {
  static_cast<NSScrollView*>(view()).hasVerticalScroller = has;
}

bool Scroll::HasVerticalScrollBar() const {
  return static_cast<NSScrollView*>(view()).hasVerticalScroller;
}

void Scroll::SetHorizontalScrollBar(bool has) {
  static_cast<NSScrollView*>(view()).hasHorizontalScroller = has;
}

bool Scroll::HasHorizontalScrollBar() const {
  return static_cast<NSScrollView*>(view()).hasHorizontalScroller;
}

void Scroll::SetAutoHideScrollBar(bool is) {
  static_cast<NSScrollView*>(view()).autohidesScrollers = is;
}

bool Scroll::IsScrollBarAutoHide() const {
  return static_cast<NSScrollView*>(view()).autohidesScrollers;
}

}  // namespace nu
