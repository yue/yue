// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Avoid compiling this file with other build systems.
#if defined(WEBVIEW2_SUPPORT)

#include "nativeui/win/webview2/browser_impl_webview2.h"

#include <string>
#include <utility>

#include "base/base_paths.h"
#include "base/base_paths_win.h"
#include "base/json/json_reader.h"
#include "base/path_service.h"
#include "base/scoped_native_library.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_co_mem.h"
#include "nativeui/app.h"
#include "nativeui/state.h"

namespace nu {

namespace {

// C:\Users\USER_NAME\AppData\Local\APPLICATION_NAME
base::FilePath GetUserDataDir() {
  base::FilePath path;
  if (!base::PathService::Get(base::DIR_LOCAL_APP_DATA, &path))
    base::PathService::Get(base::DIR_TEMP, &path);
  return path.Append(base::UTF8ToWide(App::GetCurrent()->GetName()));
}

}  // namespace

BrowserImplWebview2::BrowserImplWebview2(Browser::Options options,
                                         BrowserHolder* holder)
    : BrowserImpl(std::move(options), holder),
      weak_factory_(this) {
  base::WeakPtr<BrowserImplWebview2> self = weak_factory_.GetWeakPtr();
  auto callback =
      Microsoft::WRL::Callback<
          ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
              [self](HRESULT res, ICoreWebView2Environment* env) {
                if (!self)
                  return E_FAIL;
                return self->OnEnvCreated(res, env);
              });
  auto* create =
      reinterpret_cast<decltype(&::CreateCoreWebView2EnvironmentWithOptions)>(
          State::GetCurrent()->GetWebView2Loader()->GetFunctionPointer(
              "CreateCoreWebView2EnvironmentWithOptions"));
  if (!create ||
      FAILED(create(nullptr, GetUserDataDir().value().c_str(), nullptr,
                    callback.Get()))) {
    CreationFailed();
    return;
  }
}

BrowserImplWebview2::~BrowserImplWebview2() {
  if (controller_) {
    controller_->Close();
    controller_->remove_GotFocus(on_got_focus_);
    controller_->remove_LostFocus(on_lost_focus_);
    controller_->remove_AcceleratorKeyPressed(on_accelerator_key_pressed_);
  }
  if (webview_) {
    webview_->remove_WindowCloseRequested(on_close_);
    webview_->remove_HistoryChanged(on_history_changed_);
    webview_->remove_NavigationStarting(on_navigation_starting_);
    webview_->remove_NavigationCompleted(on_navigation_completed_);
    webview_->remove_DocumentTitleChanged(on_document_title_changed_);
    webview_->remove_SourceChanged(on_source_changed_);
    webview_->remove_WebMessageReceived(on_web_message_received_);
  }
}

bool BrowserImplWebview2::IsWebView2() const {
  return true;
}

void BrowserImplWebview2::LoadURL(std::wstring str) {
  if (!webview_)
    return;
  bool should_update_bindings = delegate()->HasBindings() && is_first_load_;
  is_first_load_ = false;
  if (should_update_bindings) {
    // The binding script is not added until the first load.
    UpdateBindings();
    // Do the load after the binding script is added.
    base::WeakPtr<BrowserImplWebview2> ref = weak_factory_.GetWeakPtr();
    pending_load_ = [ref, str]() {
      if (ref)
        ref->LoadURL(std::move(str));
    };
    return;
  }
  webview_->Navigate(str.c_str());
  ReceiveBrowserHWND();
}

void BrowserImplWebview2::LoadHTML(std::wstring str,
                                   std::wstring base_url) {
  if (!webview_)
    return;
  bool should_update_bindings = delegate()->HasBindings() && is_first_load_;
  is_first_load_ = false;
  if (should_update_bindings) {
    // The binding script is not added until the first load.
    UpdateBindings();
    // Do the load after the binding script is added.
    base::WeakPtr<BrowserImplWebview2> ref = weak_factory_.GetWeakPtr();
    pending_load_ = [ref, str, base_url]() {
      if (ref)
        ref->LoadHTML(std::move(str), std::move(base_url));
    };
    return;
  }
  // WebView2 does not support setting base URL.
  webview_->NavigateToString(str.c_str());
  ReceiveBrowserHWND();
}

std::wstring BrowserImplWebview2::GetURL() {
  base::win::ScopedCoMem<wchar_t> url;
  if (webview_)
    webview_->get_Source(&url);
  return url.get() ? url.get() : std::wstring();
}

std::wstring BrowserImplWebview2::GetTitle() {
  base::win::ScopedCoMem<wchar_t> title;
  if (webview_)
    webview_->get_DocumentTitle(&title);
  return title.get() ? title.get() : std::wstring();
}

void BrowserImplWebview2::SetUserAgent(const std::string& ua) {
  // WebView2 does not provide API to change user agent.
}

void BrowserImplWebview2::ExecuteJavaScript(
    std::wstring code,
    const Browser::ExecutionCallback& callback) {
  if (webview_) {
    auto handler =
        Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [callback](HRESULT res, LPCWSTR json) -> HRESULT {
              if (!callback)
                return S_OK;
              base::Value result;
              if (SUCCEEDED(res)) {
                auto pv = base::JSONReader::Read(base::WideToUTF8(json));
                if (pv)
                  result = std::move(*pv);
              }
              // |res| would be S_OK even when the code throws, currently there
              // is no way to know if the executaion succeeded.
              callback(SUCCEEDED(res), std::move(result));
              return S_OK;
            });
    webview_->ExecuteScript(code.c_str(), handler.Get());
  }
}

void BrowserImplWebview2::GoBack() {
  if (webview_)
    webview_->GoBack();
}

bool BrowserImplWebview2::CanGoBack() const {
  BOOL can = false;
  if (webview_)
    webview_->get_CanGoBack(&can);
  return can;
}

void BrowserImplWebview2::GoForward() {
  if (webview_)
    webview_->GoForward();
}

bool BrowserImplWebview2::CanGoForward() const {
  BOOL can = false;
  if (webview_)
    webview_->get_CanGoForward(&can);
  return can;
}

void BrowserImplWebview2::Reload() {
  if (webview_)
    webview_->Reload();
}

void BrowserImplWebview2::Stop() {
  if (webview_)
    webview_->Stop();
}

bool BrowserImplWebview2::IsLoading() const {
  return is_loading_;
}

void BrowserImplWebview2::UpdateBindings() {
  if (is_first_load_ || !delegate()->HasBindings() || !webview_)
    return;
  // Schedule another update if there is already one.
  if (is_script_adding_) {
    pending_script_update_ = true;
    return;
  }
  // Clear current script.
  if (!script_id_.empty())
    webview_->RemoveScriptToExecuteOnDocumentCreated(script_id_.c_str());
  // Add script asyncronously.
  is_script_adding_ = true;
  base::WeakPtr<BrowserImplWebview2> ref = weak_factory_.GetWeakPtr();
  auto callback =
      Microsoft::WRL::Callback<
          ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
              [ref](HRESULT res, PCWSTR id) {
                if (!ref)
                  return E_FAIL;
                BrowserImplWebview2* self = ref.get();
                self->is_script_adding_ = false;
                self->script_id_ = id;
                // Some tasks happened during the update.
                if (self->pending_script_update_) {
                  self->pending_script_update_ = false;
                  self->UpdateBindings();
                }
                if (self->pending_load_) {
                  self->pending_load_();
                  self->pending_load_ = nullptr;
                }
                return S_OK;
              });
  webview_->AddScriptToExecuteOnDocumentCreated(
      base::UTF8ToWide(delegate()->GetBindingScript()).c_str(),
      callback.Get());
}

void BrowserImplWebview2::SetBounds(RECT rect) {
  if (!controller_)
    return;
  // WebView2 might hide itself if the holder is not visible at first, make
  // sure it is shown.
  //
  // Note that we never call put_IsVisible(false) to hide the webview, because
  // there are some bug in WebView2 that calling put_IsVisible(false) would
  // trigger GotFocus/LostFocus events that mess our focus system.
  BOOL parent_visible = ::IsWindowVisible(hwnd());
  BOOL visible = true;
  controller_->get_IsVisible(&visible);
  if (parent_visible && !visible)
    controller_->put_IsVisible(true);
  // Resize the webview to sync with parent subwin.
  controller_->put_Bounds(rect);
}

void BrowserImplWebview2::Focus() {
  if (controller_)
    controller_->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
}

bool BrowserImplWebview2::HasFocus() const {
  return has_focus_;
}

void BrowserImplWebview2::OnMove() {
  if (controller_)
    controller_->NotifyParentWindowPositionChanged();
}

void BrowserImplWebview2::ReceiveBrowserHWND() {
  if (browser_hwnd())
    return;
  HWND win = ::FindWindowEx(hwnd(), nullptr, L"Chrome_WidgetWin_0", nullptr);
  if (!win)
    return;
  win = ::FindWindowEx(win, nullptr, L"Chrome_WidgetWin_1", nullptr);
  if (!win)
    return;
  holder()->ReportBrowserHWND(::FindWindowEx(
      win, nullptr, L"Chrome_RenderWidgetHostHWND", nullptr));
}

HRESULT BrowserImplWebview2::CreationFailed() {
  holder()->OnWebView2Completed(this, false);
  return E_FAIL;
}

void BrowserImplWebview2::OnReady() {
  // Set options.
  Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;
  if (SUCCEEDED(webview_->get_Settings(&settings))) {
    settings->put_AreDefaultContextMenusEnabled(options().context_menu);
    settings->put_AreDevToolsEnabled(options().devtools);
    settings->put_IsWebMessageEnabled(true);
  }
  // Register event handlers.
  controller_->add_GotFocus(
      Microsoft::WRL::Callback<
          ICoreWebView2FocusChangedEventHandler>(
              this, &BrowserImplWebview2::OnGotFocus).Get(),
      &on_got_focus_);
  controller_->add_LostFocus(
      Microsoft::WRL::Callback<
          ICoreWebView2FocusChangedEventHandler>(
              this, &BrowserImplWebview2::OnLostFocus).Get(),
      &on_lost_focus_);
  controller_->add_AcceleratorKeyPressed(
      Microsoft::WRL::Callback<
          ICoreWebView2AcceleratorKeyPressedEventHandler>(
              this, &BrowserImplWebview2::OnAcceleratorKeyPressed).Get(),
      &on_accelerator_key_pressed_);
  webview_->add_WindowCloseRequested(
      Microsoft::WRL::Callback<
          ICoreWebView2WindowCloseRequestedEventHandler>(
              this, &BrowserImplWebview2::OnClose).Get(),
      &on_close_);
  webview_->add_HistoryChanged(
      Microsoft::WRL::Callback<
          ICoreWebView2HistoryChangedEventHandler>(
              this, &BrowserImplWebview2::OnHistoryChanged).Get(),
      &on_history_changed_);
  webview_->add_NavigationStarting(
      Microsoft::WRL::Callback<
          ICoreWebView2NavigationStartingEventHandler>(
              this, &BrowserImplWebview2::OnNavigationStarting).Get(),
      &on_navigation_starting_);
  webview_->add_NavigationCompleted(
      Microsoft::WRL::Callback<
          ICoreWebView2NavigationCompletedEventHandler>(
              this, &BrowserImplWebview2::OnNavigationCompleted).Get(),
      &on_navigation_completed_);
  webview_->add_DocumentTitleChanged(
      Microsoft::WRL::Callback<
          ICoreWebView2DocumentTitleChangedEventHandler>(
              this, &BrowserImplWebview2::OnDocumentTitleChanged).Get(),
      &on_document_title_changed_);
  webview_->add_SourceChanged(
      Microsoft::WRL::Callback<
          ICoreWebView2SourceChangedEventHandler>(
              this, &BrowserImplWebview2::OnSourceChanged).Get(),
      &on_source_changed_);
  webview_->add_WebMessageReceived(
      Microsoft::WRL::Callback<
          ICoreWebView2WebMessageReceivedEventHandler>(
              this, &BrowserImplWebview2::OnWebMessageReceived).Get(),
      &on_web_message_received_);
  // Notify the holder.
  holder()->OnWebView2Completed(this, true);
}

HRESULT BrowserImplWebview2::OnEnvCreated(
    HRESULT res, ICoreWebView2Environment* env) {
  if (options().webview2_force_ie || FAILED(res))
    return CreationFailed();
  base::WeakPtr<BrowserImplWebview2> self = weak_factory_.GetWeakPtr();
  auto callback =
      Microsoft::WRL::Callback<
          ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
              [self](HRESULT res, ICoreWebView2Controller* controller) {
                if (!self)
                  return E_FAIL;
                return self->OnControllerCreated(res, controller);
              });
  if (FAILED(env->CreateCoreWebView2Controller(hwnd(), callback.Get())))
    return CreationFailed();
  env_ = env;
  return S_OK;
}

HRESULT BrowserImplWebview2::OnControllerCreated(
    HRESULT res, ICoreWebView2Controller* controller) {
  if (FAILED(res) || FAILED(controller->get_CoreWebView2(&webview_)))
    return CreationFailed();
  controller_ = controller;
  OnReady();
  return S_OK;
}

HRESULT BrowserImplWebview2::OnGotFocus(ICoreWebView2Controller* sender,
                                        IUnknown* args) {
  if (window())
    window()->focus_manager()->TakeFocus(holder());
  has_focus_ = true;
  return S_OK;
}

HRESULT BrowserImplWebview2::OnLostFocus(ICoreWebView2Controller*, IUnknown*) {
  has_focus_ = false;
  return S_OK;
}

HRESULT BrowserImplWebview2::OnAcceleratorKeyPressed(
    ICoreWebView2Controller*,
    ICoreWebView2AcceleratorKeyPressedEventArgs* args) {
  COREWEBVIEW2_KEY_EVENT_KIND kind;
  if (FAILED(args->get_KeyEventKind(&kind)))
    return S_OK;
  UINT key;
  if (FAILED(args->get_VirtualKey(&key)))
    return S_OK;
  INT l_param;
  if (FAILED(args->get_KeyEventLParam(&l_param)))
    return S_OK;
  UINT message;
  switch (kind) {
    case COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN:
      message = WM_KEYDOWN;
      break;
    case COREWEBVIEW2_KEY_EVENT_KIND_KEY_UP:
      message = WM_KEYUP;
      break;
    case COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN:
      message = WM_SYSKEYDOWN;
      break;
    case COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_UP:
      message = WM_SYSKEYUP;
      break;
  }
  // Ask if ViewImpl wants to handle the key.
  holder()->OnKeyEvent(message, key, l_param);
  if (holder()->IsMsgHandled()) {
    args->put_Handled(true);
    return S_OK;
  }
  args->put_Handled(false);
  return S_OK;
}

HRESULT BrowserImplWebview2::OnClose(ICoreWebView2* sender, IUnknown* args) {
  delegate()->on_close.Emit(delegate());
  return S_OK;
}

HRESULT BrowserImplWebview2::OnHistoryChanged(ICoreWebView2*, IUnknown*) {
  delegate()->on_update_command.Emit(delegate());
  return S_OK;
}

HRESULT BrowserImplWebview2::OnNavigationStarting(
    ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) {
  is_loading_ = true;
  delegate()->on_change_loading.Emit(delegate());

  base::win::ScopedCoMem<wchar_t> url;
  args->get_Uri(&url);
  delegate()->on_start_navigation.Emit(
      delegate(), url.get() ? base::WideToUTF8(url.get()) : std::string());
  return S_OK;
}

HRESULT BrowserImplWebview2::OnNavigationCompleted(
    ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) {
  is_loading_ = false;
  delegate()->on_change_loading.Emit(delegate());

  BOOL success = false;
  args->get_IsSuccess(&success);
  std::string url = delegate()->GetURL();
  if (success) {
    delegate()->on_finish_navigation.Emit(delegate(), url);
  } else {
    COREWEBVIEW2_WEB_ERROR_STATUS error =
        COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
    args->get_WebErrorStatus(&error);
    delegate()->on_fail_navigation.Emit(delegate(), url,
                                        static_cast<int>(error));
  }
  return S_OK;
}

HRESULT BrowserImplWebview2::OnDocumentTitleChanged(
    ICoreWebView2*, IUnknown* args) {
  delegate()->on_update_title.Emit(delegate(), delegate()->GetTitle());
  return S_OK;
}

HRESULT BrowserImplWebview2::OnSourceChanged(
    ICoreWebView2*, ICoreWebView2SourceChangedEventArgs* args) {
  delegate()->on_commit_navigation.Emit(delegate(), delegate()->GetURL());
  return S_OK;
}

HRESULT BrowserImplWebview2::OnWebMessageReceived(
    ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) {
  base::win::ScopedCoMem<wchar_t> message;
  if (FAILED(args->TryGetWebMessageAsString(&message)) || !message.get())
    return E_INVALIDARG;
  return delegate()->InvokeBindings(
     base::WideToUTF8(message.get())) ? S_OK : E_INVALIDARG;
}

}  // namespace nu

#endif  // defined(WEBVIEW2_SUPPORT)
