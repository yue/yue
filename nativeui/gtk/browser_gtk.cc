// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <JavaScriptCore/JavaScript.h>
#include <webkit2/webkit2.h>

#include "base/json/json_reader.h"
#include "nativeui/gtk/nu_protocol_stream.h"
#include "nativeui/gtk/util/widget_util.h"

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
  // TODO(zcbenz): Convert types directly instead of using JSON parsing.
  JSStringRef json = JSValueCreateJSONString(context, value, 0, nullptr);
  if (!json)
    return base::Value();
  std::string json_str = JSStringToString(json);
  JSStringRelease(json);
  absl::optional<base::Value> result = base::JSONReader::Read(json_str);
  if (!result)
    return base::Value();
  return std::move(*result);
}

gboolean OnContextMenu(WebKitWebView* widget,
                       GtkWidget *default_menu,
                       WebKitHitTestResult* hit_test_result,
                       gboolean triggered_with_keyboard,
                       Browser* view) {
  return TRUE;
}

void OnNotifyTitle(WebKitWebView*, GParamSpec*, Browser* view) {
  view->on_update_title.Emit(view, view->GetTitle());
}

void OnNotifyIsLoading(WebKitWebView*, GParamSpec*, Browser* view) {
  view->on_change_loading.Emit(view);
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

void OnBackForwadListChanged(WebKitBackForwardList* backforward_list,
                             WebKitBackForwardListItem* added,
                             GList* removed,
                             Browser *view) {
  view->on_update_command.Emit(view);
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
  auto* context = webkit_javascript_result_get_global_context(js_result);
  auto* value = webkit_javascript_result_get_value(js_result);
  if (!JSValueIsString(context, value))
    return;
  JSStringRef str = JSValueToStringCopy(context, value, nullptr);
  browser->InvokeBindings(JSStringToString(str));
  JSStringRelease(str);
}

void OnNullProtocolRequest(WebKitURISchemeRequest* request, gpointer) {
  GError* error = g_error_new_literal(
      g_quark_from_static_string("yue"),
      WEBKIT_NETWORK_ERROR_UNKNOWN_PROTOCOL,
      "The protocol has been unregistered");
  webkit_uri_scheme_request_finish_error(request, error);
  g_error_free(error);
}

void OnProtocolRequest(WebKitURISchemeRequest* request,
                       Browser::ProtocolHandler* handler) {
  // Create job.
  ProtocolJob* protocol_job =
      (*handler)(webkit_uri_scheme_request_get_uri(request));
  if (!protocol_job) {
    GError* error = g_error_new_literal(
        g_quark_from_static_string("yue"),
        WEBKIT_NETWORK_ERROR_FAILED,
        "The protocol handler did not return a request job");
    webkit_uri_scheme_request_finish_error(request, error);
    g_error_free(error);
    return;
  }
  // Manage the protocol_job with the stream.
  // DO NOT pass protocol_job to the lambda, it would cause circular ref.
  GInputStream* protocol_stream = nu_protocol_stream_new(protocol_job);
  std::string mime_type;
  protocol_job->GetMimeType(&mime_type);
  // Start.
  g_object_ref(request);
  protocol_job->Plug([protocol_stream, request, mime_type](int size) {
    webkit_uri_scheme_request_finish(
        request, protocol_stream, size,
        mime_type.empty() ? nullptr : mime_type.c_str());
    g_object_unref(protocol_stream);
    g_object_unref(request);
  });
  if (!protocol_job->Start()) {
    GError* error = g_error_new_literal(
        g_quark_from_static_string("yue"),
        WEBKIT_NETWORK_ERROR_FAILED,
        "The protocol request job failed to start");
    webkit_uri_scheme_request_finish_error(request, error);
    g_error_free(error);
    // Free on failure.
    g_object_unref(protocol_stream);
    g_object_unref(request);
  }
}

}  // namespace

void Browser::PlatformInit(Options options) {
  // Install native bindings script.
  WebKitUserContentManager* manager = webkit_user_content_manager_new();
  g_signal_connect(manager, "script-message-received::yue",
                   G_CALLBACK(OnScriptMessage), this);
  webkit_user_content_manager_register_script_message_handler(manager, "yue");

  // Create webview.
  GtkWidget* webview = webkit_web_view_new_with_user_content_manager(manager);
  TakeOverView(webview);

  // Assign a new settings to avoid affecting other webviews.
  WebKitSettings* settings = webkit_settings_new();
  // Configurations.
  if (options.devtools) {
    webkit_settings_set_enable_write_console_messages_to_stdout(settings, true);
    webkit_settings_set_enable_developer_extras(settings, true);
  }
  if (options.allow_file_access_from_files) {
    webkit_settings_set_allow_file_access_from_file_urls(settings, true);
  }
  if (!options.hardware_acceleration) {
    webkit_settings_set_hardware_acceleration_policy(
        settings, WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER);
  }
  webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webview), settings);

  // Disable the context menu.
  if (!options.context_menu) {
    g_signal_connect(webview, "context-menu", G_CALLBACK(OnContextMenu), this);
  }

  // Install events.
  g_signal_connect(webview, "notify::title", G_CALLBACK(OnNotifyTitle), this);
  g_signal_connect(webview, "notify::is-loading",
                   G_CALLBACK(OnNotifyIsLoading), this);
  g_signal_connect(webview, "close", G_CALLBACK(OnClose), this);
  g_signal_connect(webview, "load-changed", G_CALLBACK(OnLoadChanged), this);
  g_signal_connect(webview, "load-failed", G_CALLBACK(OnLoadFailed), this);
  WebKitBackForwardList* backforward_list =
      webkit_web_view_get_back_forward_list(WEBKIT_WEB_VIEW(webview));
  g_signal_connect(backforward_list, "changed",
                   G_CALLBACK(OnBackForwadListChanged), this);
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
  const char* url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(GetNative()));
  if (url)
    return url;
  else
    return std::string();
}

std::string Browser::GetTitle() {
  const char* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(GetNative()));
  if (title)
    return title;
  else
    return std::string();
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

bool Browser::CanGoBack() const {
  return webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::GoForward() {
  webkit_web_view_go_forward(WEBKIT_WEB_VIEW(GetNative()));
}

bool Browser::CanGoForward() const {
  return webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::Reload() {
  webkit_web_view_reload(WEBKIT_WEB_VIEW(GetNative()));
}

void Browser::Stop() {
  webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(GetNative()));
}

bool Browser::IsLoading() const {
  return webkit_web_view_is_loading(WEBKIT_WEB_VIEW(GetNative()));
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

// static
bool Browser::RegisterProtocol(const std::string& scheme,
                               ProtocolHandler handler) {
  WebKitWebContext* context = webkit_web_context_get_default();
  webkit_web_context_register_uri_scheme(
      context,
      scheme.c_str(),
      reinterpret_cast<WebKitURISchemeRequestCallback>(&OnProtocolRequest),
      new ProtocolHandler(std::move(handler)),
      Delete<ProtocolHandler>);
  return true;
}

// static
void Browser::UnregisterProtocol(const std::string& scheme) {
  // There is no unregister API, just replace with a handler to return error.
  WebKitWebContext* context = webkit_web_context_get_default();
  webkit_web_context_register_uri_scheme(
      context, scheme.c_str(), &OnNullProtocolRequest, nullptr, nullptr);
}

}  // namespace nu
