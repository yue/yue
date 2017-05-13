// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#import <WebKit/WebKit.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUWebView : WebView<NUView> {
 @private
  nu::NUPrivate private_;
}
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
@end

@implementation NUWebView

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
}

@end

@interface NUWebUIDelegate : NSObject<WebUIDelegate>
@end

@implementation NUWebUIDelegate

- (void)webViewClose:(WebView*)sender {
  auto* browser = static_cast<nu::Browser*>([sender shell]);
  browser->on_close.Emit(browser);
}

@end

namespace nu {

Browser::Browser() {
  NUWebView* webview = [[NUWebView alloc] initWithFrame:NSZeroRect];
  [webview setUIDelegate:[[NUWebUIDelegate alloc] init]];
  TakeOverView(webview);
}

Browser::~Browser() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [[webview UIDelegate] release];
}

void Browser::LoadURL(const std::string& url) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [webview setMainFrameURL:base::SysUTF8ToNSString(url)];
}

}  // namespace nu
