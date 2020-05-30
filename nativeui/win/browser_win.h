// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_WIN_H_
#define NATIVEUI_WIN_BROWSER_WIN_H_

#include "nativeui/browser.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

// Interface for implementing browsers.
class BrowserImpl : public SubwinView {
 public:
  BrowserImpl(Browser::Options options, Browser* delegate);

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

  const Browser::Options& options() const { return options_; }

 private:
  Browser::Options options_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_WIN_H_
