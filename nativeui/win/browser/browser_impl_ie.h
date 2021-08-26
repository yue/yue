// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_BROWSER_IMPL_IE_H_
#define NATIVEUI_WIN_BROWSER_BROWSER_IMPL_IE_H_

#include <unknwn.h>
#include <exdisp.h>  // NOLINT
#include <mshtml.h>
#include <wrl.h>

#include <string>

#include "base/memory/weak_ptr.h"
#include "nativeui/win/browser/browser_document_events.h"
#include "nativeui/win/browser/browser_event_sink.h"
#include "nativeui/win/browser/browser_external_sink.h"
#include "nativeui/win/browser/browser_html_moniker.h"
#include "nativeui/win/browser/browser_ole_site.h"
#include "nativeui/win/browser_win.h"

namespace nu {

// Implementation of Browser based on IE.
class BrowserImplIE : public BrowserImpl {
 public:
  static bool RegisterProtocol(std::wstring scheme,
                               Browser::ProtocolHandler handler);
  static void UnregisterProtocol(std::wstring scheme);

  BrowserImplIE(Browser::Options options, BrowserHolder* holder);
  ~BrowserImplIE() override;

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

  void SetBounds(RECT rect) override;
  void Focus() override;
  bool HasFocus() const override;

  bool GetScript(Microsoft::WRL::ComPtr<IDispatchEx>* out);

  template<typename T>
  bool GetBrowser(Microsoft::WRL::ComPtr<T>* out) {
    return SUCCEEDED(browser_.As(out));
  }

  void set_can_go_back(bool b) { can_go_back_ = b; }
  void set_can_go_forward(bool b) { can_go_forward_ = b; }

 protected:
  CR_BEGIN_MSG_MAP_EX(BrowserImplIE, BrowserImpl)
    CR_MSG_WM_DESTROY(OnDestroy)
    CR_MESSAGE_HANDLER_EX(WM_PARENTNOTIFY, OnParentNotify)
  CR_END_MSG_MAP()

 private:
  friend class BrowserEventSink;

  void OnDestroy();
  LRESULT OnParentNotify(UINT msg, WPARAM w_param, LPARAM l_param);

  bool Eval(std::wstring code, base::Value* result);

  // Get the HWND of the IE control and add hooks.
  void ReceiveBrowserHWND();

  // Cleanup the hooks on IE control.
  void CleanupBrowserHWND();

  // Called when document is ready.
  void OnDocumentReady();

  // The proc of IE control.
  static LRESULT CALLBACK BrowserWndProc(HWND hwnd,
                                         UINT message,
                                         WPARAM w_param,
                                         LPARAM l_param);

  Microsoft::WRL::ComPtr<BrowserExternalSink> external_sink_;
  Microsoft::WRL::ComPtr<BrowserOleSite> ole_site_;
  Microsoft::WRL::ComPtr<BrowserEventSink> event_sink_;
  Microsoft::WRL::ComPtr<BrowserDocumentEvents> document_events_;
  Microsoft::WRL::ComPtr<BrowserHTMLMoniker> html_moniker_;
  WNDPROC browser_proc_ = nullptr;

  Microsoft::WRL::ComPtr<IWebBrowser2> browser_;
  Microsoft::WRL::ComPtr<IHTMLDocument2> document_;

  // Whether we have loaded the HTML.
  bool is_html_loaded_ = false;

  // Browser states.
  bool can_go_back_ = false;
  bool can_go_forward_ = false;

  base::WeakPtrFactory<BrowserImplIE> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_BROWSER_IMPL_IE_H_
