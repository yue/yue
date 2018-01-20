// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <exdisp.h>
#include <ole2.h>
#include <wrl.h>

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"
#include "base/win/scoped_bstr.h"
#include "base/win/scoped_com_initializer.h"
#include "base/win/scoped_variant.h"
#include "nativeui/win/browser_ole_site.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

const wchar_t* IE_EMULATION_KEY =
    L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\"
    L"FEATURE_BROWSER_EMULATION";
const DWORD IE_VERSION = 11000;


// Set register key to prevent using compatible mode of IE.
void FixIECompatibleMode() {
  base::FilePath exe_path;
  if (!PathService::Get(base::FILE_EXE, &exe_path))
    return;
  base::win::RegKey(HKEY_CURRENT_USER, IE_EMULATION_KEY, KEY_ALL_ACCESS)
      .WriteValue(exe_path.BaseName().value().c_str(), IE_VERSION);
}

// Implementation of Browser.
class BrowserImpl : public SubwinView {
 public:
  explicit BrowserImpl(Browser* delegate)
      : SubwinView(delegate) {
    FixIECompatibleMode();
    Microsoft::WRL::ComPtr<IClassFactory> class_factory;
    if (FAILED(::CoGetClassObject(CLSID_WebBrowser,
                                  CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                                  nullptr,
                                  IID_PPV_ARGS(&class_factory)))) {
      PLOG(ERROR) << "Failed to call CoGetClassObject on CLSID_WebBrowser";
      return;
    }
    if (FAILED(class_factory->CreateInstance(nullptr,
                                             IID_PPV_ARGS(&browser_)))) {
      PLOG(ERROR) << "Failed to create instance on CLSID_WebBrowser";
      return;
    }
    ole_site_ = new BrowserOleSite(hwnd());
    Microsoft::WRL::ComPtr<IOleObject> ole_object;
    if (FAILED(browser_.As(&ole_object)) ||
        FAILED(ole_object->SetClientSite(ole_site_.Get()))) {
      PLOG(ERROR) << "Failed to set client site";
      return;
    }
    RECT rc = { 0 };
    ole_object->DoVerb(OLEIVERB_INPLACEACTIVATE, nullptr, ole_site_.Get(), -1,
                       hwnd(), &rc);
  }

  void LoadURL(const base::string16& str) {
    base::win::ScopedBstr url(str.c_str());
    browser_->Navigate(url, nullptr, nullptr, nullptr, nullptr);
  }

 protected:
  // ViewImpl:
  void SizeAllocate(const Rect& bounds) override {
    SubwinView::SizeAllocate(bounds);
    Microsoft::WRL::ComPtr<IOleInPlaceObject> in_place;
    if (FAILED(browser_.As(&in_place)))
      return;
    RECT rc = { 0, 0, bounds.width(), bounds.height() };
    in_place->SetObjectRects(&rc, &rc);
  }

 private:
  base::win::ScopedCOMInitializer com_;
  Microsoft::WRL::ComPtr<BrowserOleSite> ole_site_;
  Microsoft::WRL::ComPtr<IWebBrowser2> browser_;
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
