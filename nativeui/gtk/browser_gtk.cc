// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <gtk/gtk.h>
#include <JavaScriptCore/JavaScript.h>
#include <webkit2/webkit2.h>

#include "base/json/json_reader.h"

namespace nu {

namespace {

const char* kIgnoreNextFinish = "ignore-next-finish";

std::string JSStringToString(JSStringRef js) {
  size_t max_size = JSStringGetMaximumUTF8CStringSize(js);
  std::string str(max_size, '\0');
  JSStringGetUTF8CString(js, &str[0], max_size);
  size_t length = strlen(str.c_str());
  str.erase(length, max_size - length);
  return str;
}

base::Value JSResultToBaseValue(WebKitJavascriptResult* js_result) {
  auto* context = webkit_javascript_result_get_global_context(js_result);
  auto* value = webkit_javascript_result_get_value(js_result);
  // While manually iterating the value should be more efficient, their API is
  // such a pain to use.
  JSStringRef json = JSValueCreateJSONString(context, value, 0, nullptr);
  if (!json)
    return base::Value();
  std::string json_str = JSStringToString(json);
  JSStringRelease(json);
  std::unique_ptr<base::Value> result = base::JSONReader::Read(json_str);
  if (!result)
    return base::Value();
  return std::move(*result.release());
}

void OnNotifyTitle(WebKitWebView*, GParamSpec*, Browser* view) {
  view->on_update_title.Emit(view, view->GetTitle());
}

void OnClose(WebKitWebView*, Browser* view) {
  view->on_close.Emit(view);
}

void OnLoadChanged(WebKitWebView* widget, WebKitLoadEvent event,
                   Browser* view) {
  switch (event) {
    case WEBKIT_LOAD_STARTED:
      view->on_start_navigation.Emit(view, webkit_web_view_get_uri(widget));
      break;
    case WEBKIT_LOAD_COMMITTED:
      view->on_commit_navigation.Emit(view, webkit_web_view_get_uri(widget));
      break;
    case WEBKIT_LOAD_FINISHED:
      // Do not emit when navigation fails.
      if (g_object_get_data(G_OBJECT(widget), kIgnoreNextFinish)) {
        g_object_set_data(G_OBJECT(widget), kIgnoreNextFinish, nullptr);
        break;
      }
      view->on_finish_navigation.Emit(view, webkit_web_view_get_uri(widget));
      break;
    case WEBKIT_LOAD_REDIRECTED:
      break;
  }
}

gboolean OnLoadFailed(WebKitWebView* widget, WebKitLoadEvent event,
                      const gchar* url, GError* error, Browser* view) {
  view->on_fail_navigation.Emit(view, url, error->code);
  // WebKitGTK always emits "finished" event even when navigation fails, so we
  // need to ignore the next "finished" event.
  g_object_set_data(G_OBJECT(widget), kIgnoreNextFinish, view);
  return TRUE;
}

void OnJavaScriptFinish(WebKitWebView* webview,
                        GAsyncResult* result,
                        Browser::ExecutionCallback* callback) {
  if (*callback) {
    auto* js_result = webkit_web_view_run_javascript_finish(
        webview, result, nullptr);
    if (!js_result) {
      (*callback)(false, base::Value());
      return;
    }
    (*callback)(true, JSResultToBaseValue(js_result));
    webkit_javascript_result_unref(js_result);
  }
  delete callback;
}

void OnScriptMessage(WebKitUserContentManager* manager,
                     WebKitJavascriptResult* js_result,
                     Browser* browser) {
  if (browser->stop_serving() || !js_result)
    return;
  base::Value args = JSResultToBaseValue(js_result);
  if (!args.is_list() || args.GetList().size() != 3 ||
      !args.GetList()[0].is_string() ||
      !args.GetList()[1].is_string() ||
      !args.GetList()[2].is_list())
    return;
  browser->InvokeBindings(args.GetList()[0].GetString(),
                          args.GetList()[1].GetString(),
                          std::move(args.GetList()[2]));
}

}  // namespace

void Browser::PlatformInit() {
  // Install native bindings script.
  WebKitUserContentManager* manager = webkit_user_content_manager_new();
  g_signal_connect(manager, "script-message-received::yue",
                   G_CALLBACK(OnScriptMessage), this);
  webkit_user_content_manager_register_script_message_handler(manager, "yue");

  // Create webview
  GtkWidget* webview = webkit_web_view_new_with_user_content_manager(manager);
  TakeOverView(webview);

  // Assign a new settings to avoid affecting other webviews.
  webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webview), webkit_settings_new());

  // Install events.
  g_signal_connect(webview, "notify::title", G_CALLBACK(OnNotifyTitle), this);
  g_signal_connect(webview, "close", G_CALLBACK(OnClose), this);
  g_signal_connect(webview, "load-changed", G_CALLBACK(OnLoadChanged), this);
  g_signal_connect(webview, "load-failed", G_CALLBACK(OnLoadFailed), this);
}

void Browser::PlatformDestroy() {
  WebKitUserContentManager* manager =
      webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(GetNative()));
  g_object_unref(manager);
}

void Browser::LoadURL(const std::string& url) {
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(GetNative()), url.c_str());
}

void Browser::LoadHTML(const std::string& str,
                       const std::string& base_url) {
  webkit_web_view_load_html(WEBKIT_WEB_VIEW(GetNative()),
                            str.c_str(), base_url.c_str());
}

std::string Browser::GetURL() {
  return webkit_web_view_get_uri(WEBKIT_WEB_VIEW(GetNative()));
}

std::string Browser::GetTitle() {
  return webkit_web_view_get_title(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::SetUserAgent(const std::string& user_agent) {
  auto* settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(GetNative()));
  webkit_settings_set_user_agent(settings, user_agent.c_str());
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

void Browser::GoBack() {
  webkit_web_view_go_back(WEBKIT_WEB_VIEW(GetNative()));
}

bool Browser::CanGoBack() {
  return webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::GoForward() {
  webkit_web_view_go_forward(WEBKIT_WEB_VIEW(GetNative()));
}

bool Browser::CanGoForward() {
  return webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::Reload() {
  webkit_web_view_reload(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::Stop() {
  webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::PlatformUpdateBindings() {
  WebKitUserContentManager* manager =
      webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(GetNative()));
  webkit_user_content_manager_remove_all_scripts(manager);
  WebKitUserScript* script = webkit_user_script_new(
      GetBindingScript().c_str(),
      WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
      WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
      nullptr,
      nullptr);
  webkit_user_content_manager_add_script(manager, script);
  webkit_user_script_unref(script);
}

}  // namespace nu
