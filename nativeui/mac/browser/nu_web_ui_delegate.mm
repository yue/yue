// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/browser/nu_web_ui_delegate.h"

#include "nativeui/browser.h"
#include "nativeui/mac/nu_view.h"

#pragma clang diagnostic ignored "-Wunguarded-availability"

@implementation NUWebUIDelegate

- (void)webView:(WKWebView*)webview
    runJavaScriptAlertPanelWithMessage:(NSString*)message
                      initiatedByFrame:(WKFrameInfo*)frame
                     completionHandler:(void (^)(void))completionHandler {
  NSAlert* alert = [[NSAlert alloc] init];
  [alert setMessageText:message];
  [alert addButtonWithTitle:@"OK"];
  if ([webview window]) {
    [alert beginSheetModalForWindow:[webview window]
                  completionHandler:^(NSModalResponse code) {
      completionHandler();
      [alert release];
    }];
  } else {
    [[alert autorelease] runModal];
    completionHandler();
  }
}

- (void)webView:(WKWebView*)webview
    runJavaScriptConfirmPanelWithMessage:(NSString*)message
                        initiatedByFrame:(WKFrameInfo*)frame
                       completionHandler:(void (^)(BOOL))completionHandler {
  NSAlert* alert = [[NSAlert alloc] init];
  [alert setMessageText:message];
  [alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
  [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
  if ([webview window]) {
    [alert beginSheetModalForWindow:[webview window]
                  completionHandler:^(NSModalResponse code) {
      completionHandler(code == NSAlertFirstButtonReturn);
      [alert release];
    }];
  } else {
    NSModalResponse response = [[alert autorelease] runModal];
    completionHandler(response == NSAlertFirstButtonReturn);
  }
}

- (void)webView:(WKWebView*)webview
    runJavaScriptTextInputPanelWithPrompt:(NSString*)prompt
                              defaultText:(NSString*)defaultText
                         initiatedByFrame:(WKFrameInfo*)frame
                        completionHandler:(void (^)(NSString*))completionHandler {
  NSTextField* field = [[NSTextField alloc] init];
  [field setFrame:NSMakeRect(0, 0, 300, [[field cell] cellSize].height)];
  [field setStringValue:(defaultText ? : @"")];
  NSAlert* alert = [[NSAlert alloc] init];
  [alert setAccessoryView:[field autorelease]];
  [alert setMessageText:prompt];
  [alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
  [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
  if ([webview window]) {
    [alert beginSheetModalForWindow:[webview window]
                  completionHandler:^(NSModalResponse code) {
      completionHandler(code == NSAlertFirstButtonReturn ? [field stringValue]
                                                         : nil);
      [alert release];
    }];
  } else {
    NSModalResponse response = [[alert autorelease] runModal];
    completionHandler(response == NSAlertFirstButtonReturn ? [field stringValue]
                                                           : nil);
  }
}

- (void)webView:(WKWebView*)webview
    runOpenPanelWithParameters:(WKOpenPanelParameters*)parameters
              initiatedByFrame:(WKFrameInfo*)frame
             completionHandler:(void (^)(NSArray<NSURL*>*))completionHandler {
  NSOpenPanel* openPanel = [[NSOpenPanel openPanel] retain];
  [openPanel setAllowsMultipleSelection:parameters.allowsMultipleSelection];
  if ([webview window]) {
    [openPanel beginSheetModalForWindow:webview.window
                      completionHandler:^(NSInteger result) {
      completionHandler(result == NSFileHandlingPanelOKButton ? [openPanel URLs]
                                                              : nil);
      [openPanel release];
    }];
  } else {
    NSInteger result = [[openPanel autorelease] runModal];
    completionHandler(result == NSFileHandlingPanelOKButton ? [openPanel URLs]
                                                            : nil);
  }
}

- (void)webViewDidClose:(WKWebView*)webview {
  auto* browser = static_cast<nu::Browser*>([webview shell]);
  browser->on_close.Emit(browser);
}

@end
