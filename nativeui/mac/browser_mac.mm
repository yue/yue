// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

#include <WebKit/WebKit.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUWebView : WKWebView<NUView> {
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

@interface NUNavigationDelegate : NSObject<WKNavigationDelegate>
@end

@implementation NUNavigationDelegate

- (void)webView:(WKWebView*)webview didFinishNavigation:(WKNavigation*)navigation {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_finish_navigation.Emit(browser);
}

@end

@interface NUWebUIDelegate : NSObject<WKUIDelegate>
@end

@implementation NUWebUIDelegate

- (void)webViewDidClose:(WKWebView*)webview {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_close.Emit(browser);
}

@end

namespace nu {

Browser::Browser() {
  NUWebView* webview = [[NUWebView alloc] initWithFrame:NSZeroRect];
  webview.UIDelegate = [[NUWebUIDelegate alloc] init];
  webview.navigationDelegate = [[NUNavigationDelegate alloc] init];
  TakeOverView(webview);
}

Browser::~Browser() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [[webview UIDelegate] release];
  [[webview navigationDelegate] release];
}

void Browser::LoadURL(const std::string& url) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  NSURL* nsurl = [NSURL URLWithString:base::SysUTF8ToNSString(url)];
  NSURLRequest* request = [NSURLRequest requestWithURL:nsurl];
  [webview loadRequest:request];
}

void Browser::ExecuteJavaScript(const std::string& code,
                                const ExecutionCallback& callback) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  __block ExecutionCallback copied_callback = callback;
  // I don't have a good way to convert result from id to JSON, so just wrap
  // the code with JSON.stringify.
  [webview evaluateJavaScript:base::SysUTF8ToNSString("JSON.stringify(" + code + ")")
            completionHandler:^(id result, NSError* error) {
    if (error || ![result isKindOfClass:[NSString class]]) {
      copied_callback(false, "");
      return;
    }
    copied_callback(true, base::SysNSStringToUTF8(static_cast<NSString*>(result)));
  }];
}

}  // namespace nu
