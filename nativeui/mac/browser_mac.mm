// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

#include <WebKit/WebKit.h>

#include "base/mac/scoped_nsobject.h"
#include "base/memory/ptr_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

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

@interface NUScriptMessageHandler : NSObject<WKScriptMessageHandler> {
 @private
  nu::Browser* shell_;
}
- (id)initWithShell:(nu::Browser*)shell;
@end

@implementation NUScriptMessageHandler

- (id)initWithShell:(nu::Browser*)shell {
  if ((self = [super init]))
    shell_ = shell;
  return self;
}

- (void)userContentController:(WKUserContentController*)controller
      didReceiveScriptMessage:(WKScriptMessage*)message {
  if (![message.name isEqualToString:@"yue"])
    return;
  base::Value args = NSValueToBaseValue(message.body);
  if (!args.is_list() || args.GetList().size() != 3 ||
      !args.GetList()[0].is_string() ||
      !args.GetList()[1].is_string() ||
      !args.GetList()[2].is_list())
    return;
  const std::string& name = args.GetList()[1].GetString();
  shell_->InvokeBindings(name, std::move(args.GetList()[2]));
}

@end

@interface NUWebView : WKWebView<NUView> {
 @private
  nu::NUPrivate private_;
  nu::Browser* shell_;
  base::scoped_nsobject<NUScriptMessageHandler> handler_;
}
- (nu::NUPrivate*)nuPrivate;
- (void)setNUFont:(nu::Font*)font;
- (void)setNUColor:(nu::Color)color;
- (void)setNUBackgroundColor:(nu::Color)color;
- (void)updateBindings;
@end

@implementation NUWebView

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (id)initWithShell:(nu::Browser*)shell {
  shell_ = shell;
  base::scoped_nsobject<WKWebViewConfiguration> config(
      [[WKWebViewConfiguration alloc] init]);
  handler_.reset([[NUScriptMessageHandler alloc] initWithShell:shell]);
  [[config userContentController] addScriptMessageHandler:handler_.get()
                                                     name:@"yue"];
  [super initWithFrame:NSZeroRect configuration:config.get()];
  return self;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
}

- (void)updateBindings {
  auto* controller = [[self configuration] userContentController];
  [controller removeAllUserScripts];
  [controller addUserScript:[self generateUserScript]];
}

- (WKUserScript*)generateUserScript {
  std::string code = "(function(key, binding, external) {";
  std::string name = shell_->binding_name();
  if (name.empty()) {
    name = "window";
  } else {
    // window[name] = {};
    name = base::StringPrintf("window[\"%s\"]", name.c_str());
    code = name + " = {};" + code;
  }
  // Insert bindings.
  for (const auto& it : shell_->bindings()) {
    code += base::StringPrintf(
        "binding[\"%s\"] = function() {"
        "  var args = Array.prototype.slice.call(arguments);"
        "  external.postMessage([key, \"%s\", args]);"
        "};",
        it.first.c_str(), it.first.c_str());
  }
  code += base::StringPrintf("})(\"%s\", %s, %s);",
                             "security_key",
                             name.c_str(),
                             "window.webkit.messageHandlers.yue");
  return [[[WKUserScript alloc]
      initWithSource:base::SysUTF8ToNSString(code)
       injectionTime:WKUserScriptInjectionTimeAtDocumentStart
    forMainFrameOnly:YES] autorelease];
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
  NUWebView* webview = [[NUWebView alloc] initWithShell:this];
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

void Browser::UpdateBindings() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [webview updateBindings];
}

}  // namespace nu
