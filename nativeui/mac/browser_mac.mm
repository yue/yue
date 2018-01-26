// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

#include <WebKit/WebKit.h>

#include "base/memory/ptr_util.h"
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

// Convert common NSValue to base::Value.
base::Value NSValueToBaseValue(id value) {
  if (!value || [value isKindOfClass:[NSNull class]]) {
    return base::Value();
  } else if ([value isKindOfClass:[NSString class]]) {
    return base::Value(base::SysNSStringToUTF8(value));
  } else if ([value isKindOfClass:[NSNumber class]]) {
    const char* objc_type = [value objCType];
    if (strcmp(objc_type, @encode(BOOL)) == 0 ||
        strcmp(objc_type, @encode(char)) == 0)
      return base::Value(static_cast<bool>([value boolValue]));
    else if (strcmp(objc_type, @encode(double)) == 0 ||
             strcmp(objc_type, @encode(float)) == 0)
      return base::Value([value doubleValue]);
    else
      return base::Value([value intValue]);
  } else if ([value isKindOfClass:[NSArray class]]) {
    base::ListValue arr;
    arr.GetList().reserve([value count]);
    for (id item in value)
      arr.GetList().push_back(NSValueToBaseValue(item));
    return std::move(arr);
  } else if ([value isKindOfClass:[NSDictionary class]]) {
    base::DictionaryValue dict;
    for (id key in value) {
      std::string str_key = base::SysNSStringToUTF8(
          [key isKindOfClass:[NSString class]] ? key : [key description]);
      auto vval = base::MakeUnique<base::Value>(
          NSValueToBaseValue([value objectForKey:key]));
      dict.SetWithoutPathExpansion(str_key.c_str(), std::move(vval));
    }
    return std::move(dict);
  } else {
    return base::Value(base::SysNSStringToUTF8([value description]));
  }
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
    copied_callback(!error, NSValueToBaseValue(result));
  }];
}

}  // namespace nu
