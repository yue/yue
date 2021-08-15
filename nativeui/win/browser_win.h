// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_WIN_H_
#define NATIVEUI_WIN_BROWSER_WIN_H_

#include <memory>
#include <string>

#include "nativeui/browser.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

class BrowserImpl;

// The window that holds the browser.
class BrowserHolder : public SubwinView {
 public:
  BrowserHolder(Browser::Options options, Browser* delegate);
  ~BrowserHolder() override;

  void ReportBrowserHWND(HWND hwnd);
#if defined(WEBVIEW2_SUPPORT)
  void OnWebView2Completed(BrowserImpl* sender, bool success);
#endif

  bool browser_created() const { return browser_created_; }
  HWND browser_hwnd() const { return browser_hwnd_; }
  BrowserImpl* impl() const { return impl_.get(); }

  Browser* delegate() { return static_cast<Browser*>(SubwinView::delegate()); }

 protected:
  // ViewImpl:
  void SizeAllocate(const Rect& bounds) override;
  void SetFocus(bool focus) override;
  bool HasFocus() const override;
  bool OnMouseWheel(NativeEvent event) override;

  // SubwinView:
  LRESULT OnMouseWheelFromSelf(
      UINT message, WPARAM w_param, LPARAM l_param) override;
  bool ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM w_param,
                            LPARAM l_param,
                            LRESULT* result) override;

 private:
  bool browser_created_ = false;
  HWND browser_hwnd_ = NULL;

  std::unique_ptr<BrowserImpl> impl_;
};

// Interface for implementing browsers.
class BrowserImpl {
 public:
  BrowserImpl(Browser::Options options, BrowserHolder* holder);
  virtual ~BrowserImpl();

  virtual bool IsWebView2() const;

  virtual void LoadURL(std::wstring str) = 0;
  virtual void LoadHTML(std::wstring str, std::wstring base_url) = 0;
  virtual std::wstring GetURL() = 0;
  virtual std::wstring GetTitle() = 0;
  virtual void SetUserAgent(const std::string& user_agent) = 0;
  virtual void ExecuteJavaScript(
      std::wstring code,
      const Browser::ExecutionCallback& callback) = 0;

  virtual void GoBack() = 0;
  virtual bool CanGoBack() const = 0;
  virtual void GoForward() = 0;
  virtual bool CanGoForward() const = 0;
  virtual void Reload() = 0;
  virtual void Stop() = 0;
  virtual bool IsLoading() const = 0;

  virtual void UpdateBindings() {}
  virtual void SetBounds(RECT rect) = 0;
  virtual void Focus() = 0;
  virtual bool HasFocus() const = 0;
  virtual void OnMove() {}

  virtual bool ProcessWindowMessage(HWND window,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param,
                                    LRESULT* result);

  Browser::Options& options() { return options_; }
  BrowserHolder* holder() { return holder_; }
  Browser* delegate() { return holder_->delegate(); }
  HWND browser_hwnd() const { return holder_->browser_hwnd(); }

  WindowImpl* window() { return holder_->window(); }
  HWND hwnd() { return holder_->hwnd(); }

 private:
  Browser::Options options_;
  BrowserHolder* holder_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_WIN_H_
