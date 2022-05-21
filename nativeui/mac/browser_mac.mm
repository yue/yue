// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

#include <WebKit/WebKit.h>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/mac/browser/nu_custom_protocol.h"
#include "nativeui/mac/browser/nu_web_ui_delegate.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"
#include "nativeui/mac/value_conversion.h"

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
  if (![message.body isKindOfClass:[NSString class]])
    return;
  shell_->InvokeBindings(base::SysNSStringToUTF8(message.body));
}

@end

@interface NUWebView : WKWebView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
  nu::Browser::Options options_;
  nu::Browser* shell_;
  base::scoped_nsobject<NUScriptMessageHandler> handler_;
}
- (id)initWithShell:(nu::Browser*)shell
            options:(nu::Browser::Options)options;
- (void)willDestroy;
- (void)updateBindings;
@end

@implementation NUWebView

- (id)initWithShell:(nu::Browser*)shell
            options:(nu::Browser::Options)options {
  options_ = std::move(options);
  shell_ = shell;
  // Initialize with configuration.
  handler_.reset([[NUScriptMessageHandler alloc] initWithShell:shell]);
  base::scoped_nsobject<WKWebViewConfiguration> config(
      [[WKWebViewConfiguration alloc] init]);
  if (options.devtools)
    [[config preferences] setValue:@YES forKey:@"developerExtrasEnabled"];
  if (options.allow_file_access_from_files)
    [[config preferences] setValue:@YES forKey:@"allowFileAccessFromFileURLs"];
  [[config userContentController] addScriptMessageHandler:handler_.get()
                                                     name:@"yue"];
  [super initWithFrame:NSZeroRect configuration:config.get()];

  // Watch the webview events
  [self addObserver:self
         forKeyPath:@"title"
            options:NSKeyValueObservingOptionNew
            context:nil];
  [self addObserver:self
         forKeyPath:@"loading"
            options:NSKeyValueObservingOptionNew
            context:nil];
  [self addObserver:self
         forKeyPath:@"canGoBack"
            options:NSKeyValueObservingOptionNew
            context:nil];
  [self addObserver:self
         forKeyPath:@"canGoForward"
            options:NSKeyValueObservingOptionNew
            context:nil];
  return self;
}

- (void)willDestroy {
  // Unsubscribe before destroying the webview.
  [self removeObserver:self forKeyPath:@"title"];
  [self removeObserver:self forKeyPath:@"loading"];
  [self removeObserver:self forKeyPath:@"canGoBack"];
  [self removeObserver:self forKeyPath:@"canGoForward"];
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

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
  if ([keyPath isEqualToString:@"title"])
    shell_->on_update_title.Emit(shell_, shell_->GetTitle());
  else if ([keyPath isEqualToString:@"loading"])
    shell_->on_change_loading.Emit(shell_);
  else if ([keyPath isEqualToString:@"canGoBack"] ||
           [keyPath isEqualToString:@"canGoForward"])
    shell_->on_update_command.Emit(shell_);
  else
    [super observeValueForKeyPath:keyPath
                         ofObject:object
                           change:change
                          context:context];
}

- (void)willOpenMenu:(NSMenu*)menu withEvent:(NSEvent*)event {
  if (options_.context_menu)
    [super willOpenMenu:menu withEvent:event];
  else
    [menu removeAllItems];
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

void Browser::PlatformInit(Options options) {
  NUWebView* webview = [[NUWebView alloc] initWithShell:this
                                                options:std::move(options)];
  [webview setUIDelegate:[[NUWebUIDelegate alloc] init]];
  [webview setNavigationDelegate:[[NUNavigationDelegate alloc] init]];
  TakeOverView(webview);
  SetWantsLayer(true);
  // Parents and siblings of Browser should have wantsLayer.
  [webview nuPrivate]->wants_layer_infected = true;
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

bool Browser::IsMagnifiable() const {
  auto* webview = static_cast<NUWebView*>(GetNative());
  return [webview allowsMagnification];
}

void Browser::SetMagnifiable(bool magnifiable) {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [webview setAllowsMagnification:magnifiable];
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

bool Browser::CanGoBack() const {
  return [static_cast<NUWebView*>(GetNative()) canGoBack];
}

void Browser::GoForward() {
  [static_cast<NUWebView*>(GetNative()) goForward:nil];
}

bool Browser::CanGoForward() const {
  return [static_cast<NUWebView*>(GetNative()) canGoForward];
}

void Browser::Reload() {
  [static_cast<NUWebView*>(GetNative()) reload:nil];
}

void Browser::Stop() {
  [static_cast<NUWebView*>(GetNative()) stopLoading:nil];
}

bool Browser::IsLoading() const {
  return [static_cast<NUWebView*>(GetNative()) isLoading];
}

void Browser::PlatformUpdateBindings() {
  auto* webview = static_cast<NUWebView*>(GetNative());
  [webview updateBindings];
}

void Browser::PlatformInstallMouseClickEvents() {
  // WKWebView does not like having mouse event handlers installed.
}

void Browser::PlatformInstallMouseMoveEvents() {
  // WKWebView does not like having mouse event handlers installed.
}

// static
bool Browser::RegisterProtocol(const std::string& scheme,
                               ProtocolHandler handler) {
  return [NUCustomProtocol registerProtocol:base::SysUTF8ToNSString(scheme)
                                withHandler:std::move(handler)];
}

// static
void Browser::UnregisterProtocol(const std::string& scheme) {
  [NUCustomProtocol unregisterProtocol:base::SysUTF8ToNSString(scheme)];
}

}  // namespace nu
