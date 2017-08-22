// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/chrome_view_mac.h"

#include <node_buffer.h>

namespace node_yue {

// static
const char ChromeView::kClassName[] = "ChromeView";

ChromeView::ChromeView(v8::Local<v8::Value> buf) {
  if (node::Buffer::HasInstance(buf) &&
      node::Buffer::Length(buf) == sizeof(NSView*)) {
    NSView* view = *reinterpret_cast<NSView**>(node::Buffer::Data(buf));
    TakeOverView(view);
  } else {
    // Show a warning text as fallback.
    scoped_refptr<nu::Label> label(new nu::Label("Invalid WebContentsView"));
    label->SetColor(nu::Color(255, 255, 255));
    label->SetBackgroundColor(nu::Color(255, 0, 0));
    // Retain the view form nu::Label and use it as ChromeView.
    NSView* view = label->GetNative();
    [view retain];
    TakeOverView(view);
  }
}

ChromeView::~ChromeView() {
}

const char* ChromeView::GetClassName() const {
  return kClassName;
}

}  // namespace node_yue
