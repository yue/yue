// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_WIN_H_
#define NATIVEUI_WIN_BROWSER_WIN_H_

#include <memory>

#include "nativeui/browser.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

class BrowserImpl;

// The window that holds the browser.
class BrowserHolder : public SubwinView {
 public:
  BrowserHolder(Browser::Options options, Browser* delegate);
  ~BrowserHolder() override;

#if defined(WEBVIEW2_SUPPORT)
  void OnWebView2Completed(bool success);
#endif

  BrowserImpl* impl() const { return impl_.get(); }

 protected:
  // ViewImpl:
  void SizeAllocate(const Rect& bounds) override;
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
  std::unique_ptr<BrowserImpl> impl_;
};

// Interface for implementing browsers.
class BrowserImpl {
 public:
  BrowserImpl(Browser::Options options,
              BrowserHolder* holder,
              Browser* delegate);
  virtual ~BrowserImpl();

  virtual void LoadURL(base::string16 str) = 0;
  virtual void LoadHTML(base::string16 str, base::string16 base_url) = 0;
  virtual base::string16 GetURL() = 0;
  virtual base::string16 GetTitle() = 0;
  virtual bool Eval(base::string16 code, base::string16* result) = 0;

  virtual void GoBack() = 0;
  virtual bool CanGoBack() const = 0;
  virtual void GoForward() = 0;
  virtual bool CanGoForward() const = 0;
  virtual void Reload() = 0;
  virtual void Stop() = 0;
  virtual bool IsLoading() const = 0;

  virtual void SetBounds(RECT rect) = 0;
  virtual bool HasFocus() const = 0;
  virtual bool OnMouseWheel(NativeEvent event) = 0;

  virtual bool ProcessWindowMessage(HWND window,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param,
                                    LRESULT* result);

  Browser::Options& options() { return options_; }
  BrowserHolder* holder() { return holder_; }
  Browser* delegate() { return delegate_; }

  WindowImpl* window() { return holder_->window(); }
  HWND hwnd() { return holder_->hwnd(); }

 private:
  Browser::Options options_;
  BrowserHolder* holder_;
  Browser* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_WIN_H_
