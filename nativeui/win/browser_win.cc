// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <exdisp.h>
#include <ole2.h>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_bstr.h"
#include "base/win/scoped_com_initializer.h"
#include "base/win/scoped_comptr.h"
#include "base/win/scoped_variant.h"
#include "nativeui/win/view_win.h"

namespace nu {

namespace {

class BrowserImpl : public ViewImpl {
 public:
  explicit BrowserImpl(Browser* delegate)
      : ViewImpl(ControlType::Browser, delegate) {
    browser_.CreateInstance(CLSID_InternetExplorer, nullptr,
                            CLSCTX_LOCAL_SERVER);
    browser_->put_Visible(VARIANT_TRUE);
  }

  void LoadURL(const base::string16& str) {
    base::win::ScopedBstr url(str.c_str());
    base::win::ScopedVariant empty_variant;
    browser_->Navigate(url, empty_variant.AsInput(), empty_variant.AsInput(),
                       empty_variant.AsInput(), empty_variant.AsInput());
  }

 private:
  base::win::ScopedCOMInitializer com_;
  base::win::ScopedComPtr<IWebBrowser2, &IID_IWebBrowser2> browser_;
};

}  // namespace

Browser::Browser() {
  TakeOverView(new BrowserImpl(this));
}

Browser::~Browser() {
}

void Browser::LoadURL(const std::string& url) {
  static_cast<BrowserImpl*>(GetNative())->LoadURL(base::UTF8ToUTF16(url));
}

}  // namespace nu
