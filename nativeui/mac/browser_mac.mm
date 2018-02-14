// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

#include <WebKit/WebKit.h>

#include "base/mac/scoped_nsobject.h"
#include "base/memory/ptr_util.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/browser/nu_custom_protocol.h"
#include "nativeui/mac/browser/nu_web_ui_delegate.h"
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
  if (shell_->stop_serving() || ![message.name isEqualToString:@"yue"])
    return;
  base::Value args = NSValueToBaseValue(message.body);
  if (!args.is_list() || args.GetList().size() != 3 ||
      !args.GetList()[0].is_string() ||
      !args.GetList()[1].is_string() ||
      !args.GetList()[2].is_list())
    return;
  shell_->InvokeBindings(args.GetList()[0].GetString(),
                         args.GetList()[1].GetString(),
                         std::move(args.GetList()[2]));
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

- (id)initWithShell:(nu::Browser*)shell {
  shell_ = shell;
  // Initialize with configuration.
  base::scoped_nsobject<WKWebViewConfiguration> config(
      [[WKWebViewConfiguration alloc] init]);
  handler_.reset([[NUScriptMessageHandler alloc] initWithShell:shell]);
  [[config userContentController] addScriptMessageHandler:handler_.get()
                                                     name:@"yue"];
  [super initWithFrame:NSZeroRect configuration:config.get()];

  // Watch the change of "title".
  [self addObserver:self
         forKeyPath:@"title"
            options:NSKeyValueObservingOptionNew
            context:nil];
  return self;
}

- (void)willDestroy {
  // Unsubscribe before destroying the webview.
  [self removeObserver:self forKeyPath:@"title"];
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
  if (![keyPath isEqualToString:@"title"] || object != self) {
    [super observeValueForKeyPath:keyPath
                         ofObject:object
                           change:change
                          context:context];
    return;
  }
  shell_->on_update_title.Emit(shell_, shell_->GetTitle());
}

- (nu::NUPrivate*)nuPrivate {
  return &private_;
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
  return [[[WKUserScript alloc]
      initWithSource:base::SysUTF8ToNSString(shell_->GetBindingScript())
       injectionTime:WKUserScriptInjectionTimeAtDocumentStart
    forMainFrameOnly:YES] autorelease];
}

@end

@interface NUNavigationDelegate : NSObject<WKNavigationDelegate>
@end

@implementation NUNavigationDelegate

- (void)webView:(WKWebView*)webview
    didStartProvisionalNavigation:(WKNavigation*)navigation {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_start_navigation.Emit(
      browser,
      base::SysNSStringToUTF8([[webview URL] absoluteString]));
}

- (void)webView:(WKWebView*)webview
    didCommitNavigation:(WKNavigation*)navigation {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_commit_navigation.Emit(
      browser,
      base::SysNSStringToUTF8([[webview URL] absoluteString]));
}

- (void)webView:(WKWebView*)webview
    didFinishNavigation:(WKNavigation*)navigation {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_finish_navigation.Emit(
      browser,
      base::SysNSStringToUTF8([[webview URL] absoluteString]));
}

- (void)webView:(WKWebView*)webview
    didFailNavigation:(WKNavigation*)navigation
            withError:(NSError*)error {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_fail_navigation.Emit(
      browser,
      base::SysNSStringToUTF8(error.userInfo[NSURLErrorFailingURLStringErrorKey]),
      [error code]);
}

- (void)webView:(WKWebView*)webview
    didFailProvisionalNavigation:(WKNavigation*)navigation
                       withError:(NSError*)error {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_fail_navigation.Emit(
      browser,
      base::SysNSStringToUTF8(error.userInfo[NSURLErrorFailingURLStringErrorKey]),
      [error code]);
}

@end

namespace nu {

void Browser::PlatformInit() {
  NUWebView* webview = [[NUWebView alloc] initWithShell:this];
  [webview setUIDelegate:[[NUWebUIDelegate alloc] init]];
  [webview setNavigationDelegate:[[NUNavigationDelegate alloc] init]];
  [webview nuPrivate]->wants_layer = true;
  [webview setWantsLayer:YES];
  TakeOverView(webview);
}

void Browser::PlatformDestroy() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [[webview UIDelegate] release];
  [[webview navigationDelegate] release];
  [webview setUIDelegate:nil];
  [webview setNavigationDelegate:nil];
  [webview willDestroy];
}

void Browser::LoadURL(const std::string& url) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  NSURL* nsurl = [NSURL URLWithString:base::SysUTF8ToNSString(url)];
  NSURLRequest* request = [NSURLRequest requestWithURL:nsurl];
  [webview loadRequest:request];
}

void Browser::LoadHTML(const std::string& str,
                       const std::string& base_url) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  NSURL* nsurl = [NSURL URLWithString:base::SysUTF8ToNSString(base_url)];
  [webview loadHTMLString:base::SysUTF8ToNSString(str)
                  baseURL:nsurl];
}

std::string Browser::GetURL() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  return base::SysNSStringToUTF8([[webview URL] absoluteString]);
}

std::string Browser::GetTitle() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  return base::SysNSStringToUTF8([webview title]);
}

void Browser::SetUserAgent(const std::string& user_agent) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [webview setCustomUserAgent:base::SysUTF8ToNSString(user_agent)];
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

void Browser::GoBack() {
  [static_cast<NUWebView*>(GetNative()) goBack:nil];
}

bool Browser::CanGoBack() {
  return [static_cast<NUWebView*>(GetNative()) canGoBack];
}

void Browser::GoForward() {
  [static_cast<NUWebView*>(GetNative()) goForward:nil];
}

bool Browser::CanGoForward() {
  return [static_cast<NUWebView*>(GetNative()) canGoForward];
}

void Browser::Reload() {
  [static_cast<NUWebView*>(GetNative()) reload:nil];
}

void Browser::Stop() {
  [static_cast<NUWebView*>(GetNative()) stopLoading:nil];
}

void Browser::PlatformUpdateBindings() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [webview updateBindings];
}

// static
bool Browser::RegisterProtocol(const std::string& scheme,
                               const ProtocolHandler& handler) {
  return [NUCustomProtocol registerProtocol:base::SysUTF8ToNSString(scheme)
                                withHandler:handler];
}

// static
void Browser::UnregisterProtocol(const std::string& scheme) {
  [NUCustomProtocol unregisterProtocol:base::SysUTF8ToNSString(scheme)];
}

}  // namespace nu
