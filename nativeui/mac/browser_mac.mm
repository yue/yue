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

namespace {

// Convert NSValue to JSON string.
std::string NSValueToJSON(id value) {
  // Try the default method.
  if ([NSJSONSerialization isValidJSONObject:value]) {
    NSData* json_data = [NSJSONSerialization dataWithJSONObject:value
                                                        options:0
                                                          error:nil];
    return std::string(static_cast<const char*>([json_data bytes]),
                       [json_data length]);
  }
  // For other types try to put it into an Array and then strip.
  NSArray* arr = @[ value ];
  if ([NSJSONSerialization isValidJSONObject:arr]) {
    std::string result =  NSValueToJSON(arr);
    return result.substr(1, result.size() - 2);
  }
  return "undefined";
}

}  // namespace

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
  if (!callback) {
    // Shortcut when no callback is passed.
    [webview evaluateJavaScript:base::SysUTF8ToNSString(code)
              completionHandler:nil];
    return;
  }

  __block ExecutionCallback copied_callback = callback;
  [webview evaluateJavaScript:base::SysUTF8ToNSString(code)
            completionHandler:^(id result, NSError* error) {
    copied_callback(!error, result ? NSValueToJSON(result) : "");
  }];
}

}  // namespace nu
