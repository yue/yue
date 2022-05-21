// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/chrome_view_mac.h"

#include <node_buffer.h>

#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUChromeView : NSView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
  scoped_refptr<nu::AttributedText> text_;
}
@end

@implementation NUChromeView

- (void)drawRect:(NSRect)dirtyRect {
  if ([[self subviews] count])
    return;

  if (!text_) {
    text_ = new nu::AttributedText(
        "No WebContents found",
        nu::TextAttributes(nu::Font::Default(), nu::Color(255, 0, 0),
                           nu::TextAlign::Center, nu::TextAlign::Center));
  }

  nu::PainterMac painter(self);
  painter.SetColor(nu::Color(0xFF, 0xFF, 0xFF));
  painter.FillRect(nu::RectF(dirtyRect));
  painter.DrawAttributedText(text_.get(),
                             nu::RectF(nu::SizeF([self frame].size)));
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
  return YES;
}

@end

namespace node_yue {

// static
const char ChromeView::kClassName[] = "ChromeView";

ChromeView::ChromeView(v8::Local<v8::Value> buf) {
  NSView* view = [[NUChromeView alloc] init];
  if (node::Buffer::HasInstance(buf) &&
      node::Buffer::Length(buf) == sizeof(NSView*)) {
    NSView* page = *reinterpret_cast<NSView**>(node::Buffer::Data(buf));
    [view addSubview:page];
  }
  TakeOverView(view);
}

ChromeView::~ChromeView() {
}

const char* ChromeView::GetClassName() const {
  return kClassName;
}

}  // namespace node_yue
