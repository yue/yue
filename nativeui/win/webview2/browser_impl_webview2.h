// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_WIN_WEBVIEW2_BROWSER_IMPL_WEBVIEW2_H_
#define NATIVEUI_WIN_WEBVIEW2_BROWSER_IMPL_WEBVIEW2_H_

#include <unknwn.h>
#include <exdisp.h>  // NOLINT
#include <webview2.h>
#include <wrl.h>

#include "nativeui/win/browser_win.h"

namespace nu {

class BrowserImplWebview2 : public BrowserImpl {
 public:
  BrowserImplWebview2(Browser::Options options,
                      BrowserHolder* holder,
                      Browser* delegate);
  ~BrowserImplWebview2() override;

  void LoadURL(base::string16 str) override;
  void LoadHTML(base::string16 str, base::string16 base_url) override;
  base::string16 GetURL() override;
  base::string16 GetTitle() override;
  bool Eval(base::string16 code, base::string16* result) override;

  void GoBack() override;
  bool CanGoBack() const override;
  void GoForward() override;
  bool CanGoForward() const override;
  void Reload() override;
  void Stop() override;
  bool IsLoading() const override;

  void SetBounds(RECT rect) override;
  bool HasFocus() const override;
  bool OnMouseWheel(NativeEvent event) override;

 private:
  HRESULT OnEnvCreated(HRESULT res, ICoreWebView2Environment* env);
  HRESULT OnControllerCreated(HRESULT res, ICoreWebView2Controller* controller);

  Microsoft::WRL::ComPtr<ICoreWebView2Environment> env_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WEBVIEW2_BROWSER_IMPL_WEBVIEW2_H_
