// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_WIN_WEBVIEW2_BROWSER_IMPL_WEBVIEW2_H_
#define NATIVEUI_WIN_WEBVIEW2_BROWSER_IMPL_WEBVIEW2_H_

#include <unknwn.h>
#include <exdisp.h>  // NOLINT
#include <webview2.h>
#include <wrl.h>

#include <string>

#include "base/memory/weak_ptr.h"
#include "nativeui/win/browser_win.h"

namespace nu {

class BrowserImplWebview2 : public BrowserImpl {
 public:
  BrowserImplWebview2(Browser::Options options, BrowserHolder* holder);
  ~BrowserImplWebview2() override;

  bool IsWebView2() const override;

  void LoadURL(std::wstring str) override;
  void LoadHTML(std::wstring str, std::wstring base_url) override;
  std::wstring GetURL() override;
  std::wstring GetTitle() override;
  void SetUserAgent(const std::string& user_agent) override;
  void ExecuteJavaScript(
      std::wstring code,
      const Browser::ExecutionCallback& callback) override;

  void GoBack() override;
  bool CanGoBack() const override;
  void GoForward() override;
  bool CanGoForward() const override;
  void Reload() override;
  void Stop() override;
  bool IsLoading() const override;

  void UpdateBindings() override;
  void SetBounds(RECT rect) override;
  void Focus() override;
  bool HasFocus() const override;
  void OnMove() override;

 private:
  void ReceiveBrowserHWND();
  HRESULT CreationFailed();
  void OnReady();

  HRESULT OnEnvCreated(HRESULT res, ICoreWebView2Environment* env);
  HRESULT OnControllerCreated(HRESULT res, ICoreWebView2Controller* controller);
  HRESULT OnGotFocus(ICoreWebView2Controller* sender, IUnknown* args);
  HRESULT OnLostFocus(ICoreWebView2Controller* sender, IUnknown* args);
  HRESULT OnAcceleratorKeyPressed(ICoreWebView2Controller*,
                                  ICoreWebView2AcceleratorKeyPressedEventArgs*);
  HRESULT OnClose(ICoreWebView2* sender, IUnknown* args);
  HRESULT OnHistoryChanged(ICoreWebView2* sender, IUnknown* args);
  HRESULT OnNavigationStarting(ICoreWebView2*,
                               ICoreWebView2NavigationStartingEventArgs*);
  HRESULT OnNavigationCompleted(ICoreWebView2*,
                                ICoreWebView2NavigationCompletedEventArgs*);
  HRESULT OnDocumentTitleChanged(ICoreWebView2* sender, IUnknown* args);
  HRESULT OnSourceChanged(ICoreWebView2*,
                          ICoreWebView2SourceChangedEventArgs*);
  HRESULT OnWebMessageReceived(ICoreWebView2* sender,
                               ICoreWebView2WebMessageReceivedEventArgs* args);

  // Whether this webview has loaded anything.
  bool is_first_load_ = true;
  // Whether current binding script is still being added.
  bool is_script_adding_ = false;
  // Whether the binding script should be updated.
  bool pending_script_update_ = false;
  // The ID of current binding script.
  std::wstring script_id_;
  // The pending operations when the binding script is being added.
  std::function<void()> pending_load_;

  bool has_focus_ = false;
  bool is_loading_ = false;

  EventRegistrationToken on_got_focus_;
  EventRegistrationToken on_lost_focus_;
  EventRegistrationToken on_accelerator_key_pressed_;
  EventRegistrationToken on_close_;
  EventRegistrationToken on_history_changed_;
  EventRegistrationToken on_navigation_starting_;
  EventRegistrationToken on_navigation_completed_;
  EventRegistrationToken on_document_title_changed_;
  EventRegistrationToken on_source_changed_;
  EventRegistrationToken on_web_message_received_;

  Microsoft::WRL::ComPtr<ICoreWebView2Environment> env_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;

  base::WeakPtrFactory<BrowserImplWebview2> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WEBVIEW2_BROWSER_IMPL_WEBVIEW2_H_
