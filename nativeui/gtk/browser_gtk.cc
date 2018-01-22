// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>

namespace nu {

namespace {

std::string JSStringToString(JSStringRef js) {
  size_t max_size = JSStringGetMaximumUTF8CStringSize(js);
  std::string str(max_size, '\0');
  JSStringGetUTF8CString(js, &str[0], max_size);
  size_t length = strlen(str.c_str());
  str.erase(length, max_size - length);
  return str;
}

void OnClose(WebKitWebView* widget, Browser* view) {
  view->on_close.Emit(view);
}

void OnLoadChanged(WebKitWebView*, WebKitLoadEvent event, Browser* view) {
  switch (event) {
    case WEBKIT_LOAD_STARTED:
      break;
    case WEBKIT_LOAD_REDIRECTED:
      break;
    case WEBKIT_LOAD_COMMITTED:
      break;
    case WEBKIT_LOAD_FINISHED:
      view->on_finish_navigation.Emit(view);
      break;
  }
}

void OnJavaScriptFinish(WebKitWebView* webview,
                        GAsyncResult* result,
                        Browser::ExecutionCallback* callback) {
  if (*callback) {
    auto* js_result = webkit_web_view_run_javascript_finish(
        webview, result, nullptr);
    if (js_result) {
      auto* context = webkit_javascript_result_get_global_context(js_result);
      auto* value = webkit_javascript_result_get_value(js_result);
      auto* json = JSValueCreateJSONString(context, value, 0, nullptr);
      if (json) {
        (*callback)(true, JSStringToString(json));
        JSStringRelease(json);
        return;
      }
      webkit_javascript_result_unref(js_result);
    }
    (*callback)(false, "");
  }
  delete callback;
}

}  // namespace

Browser::Browser() {
  GtkWidget* webview = webkit_web_view_new();
  TakeOverView(webview);

  // Install events.
  g_signal_connect(webview, "close", G_CALLBACK(OnClose), this);
  g_signal_connect(webview, "load-changed", G_CALLBACK(OnLoadChanged), this);
}

Browser::~Browser() {
}

void Browser::LoadURL(const std::string& url) {
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(GetNative()), url.c_str());
}

void Browser::ExecuteJavaScript(const std::string& code,
                                const ExecutionCallback& callback) {
  webkit_web_view_run_javascript(
      WEBKIT_WEB_VIEW(GetNative()),
      code.c_str(),
      nullptr,
      reinterpret_cast<GAsyncReadyCallback>(&OnJavaScriptFinish),
      new ExecutionCallback(callback));
}

}  // namespace nu
