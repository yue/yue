// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser_win.h"

#include <mshtml.h>
#include <shlguid.h>

#include <string>

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"
#include "base/win/scoped_bstr.h"
#include "base/win/scoped_variant.h"
#include "nativeui/message_loop.h"
#include "nativeui/state.h"
#include "nativeui/win/util/dispatch_invoke.h"
#include "nativeui/win/util/hwnd_util.h"

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

// Convert a VARIANT to JSON string.
bool VARIANTToJSON(IDispatchEx* script,
                   const base::win::ScopedVariant& value,
                   base::string16* result) {
  // Find the javascript JSON object.
  base::win::ScopedVariant json_var(base::win::ScopedVariant::kEmptyVariant);
  if (!Invoke(script, L"JSON", DISPATCH_PROPERTYGET, &json_var))
    return false;
  Microsoft::WRL::ComPtr<IDispatch> json_disp =
      static_cast<IDispatch*>(json_var.ptr()->pdispVal);
  Microsoft::WRL::ComPtr<IDispatchEx> json_obj;
  if (FAILED(json_disp.As(&json_obj)) || !json_obj)
    return false;
  // Invoke the JSON.stringify method.
  base::win::ScopedVariant str(base::win::ScopedVariant::kEmptyVariant);
  if (!Invoke(json_obj.Get(), L"stringify", DISPATCH_METHOD, &str, value))
    return false;
  *result = str.ptr()->bstrVal;
  return true;
}

}  // namespace

BrowserImpl::BrowserImpl(Browser* delegate)
    : SubwinView(delegate),
      ole_site_(new BrowserOleSite(hwnd())),
      event_sink_(new BrowserEventSink(this)) {
  set_focusable(true);
  // Initialize COM and OLE.
  State::GetCurrent()->InitializeCOM();
  // Use the latest IE version.
  FixIECompatibleMode();

  // Boring work of creating IE control.
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
  Microsoft::WRL::ComPtr<IConnectionPointContainer> cpc;
  Microsoft::WRL::ComPtr<IConnectionPoint> cp;
  DWORD cookie;
  if (FAILED(browser_.As(&cpc)) ||
      FAILED(cpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &cp)) ||
      FAILED(cp->Advise(event_sink_.Get(), &cookie))) {
    PLOG(ERROR) << "Failed to set event sink";
    return;
  }
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

BrowserImpl::~BrowserImpl() {
  CleanupBrowserHWND();
}

void BrowserImpl::LoadURL(const base::string16& str) {
  if (!browser_)
    return;
  base::win::ScopedBstr url(str.c_str());
  browser_->Navigate(url, nullptr, nullptr, nullptr, nullptr);
  ReceiveBrowserHWND();
}

bool BrowserImpl::Eval(const base::string16& code, base::string16* result) {
  if (!browser_)
    return false;
  Microsoft::WRL::ComPtr<IDispatch> doc2_disp;
  if (FAILED(browser_->get_Document(&doc2_disp)) || !doc2_disp)
    return false;
  Microsoft::WRL::ComPtr<IHTMLDocument2> doc2;
  if (FAILED(doc2_disp.As(&doc2)))
    return false;
  Microsoft::WRL::ComPtr<IDispatch> script_disp;
  if (FAILED(doc2->get_Script(&script_disp)) || !script_disp)
    return false;
  Microsoft::WRL::ComPtr<IDispatchEx> script;
  if (FAILED(script_disp.As(&script)))
    return false;
  base::win::ScopedVariant arg(code.c_str(), static_cast<UINT>(code.length()));
  base::win::ScopedVariant ret(base::win::ScopedVariant::kEmptyVariant);
  if (!Invoke(script.Get(), L"eval", DISPATCH_METHOD, &ret, arg))
    return false;
  if (!VARIANTToJSON(script.Get(), ret, result))
    return false;
  return true;
}

void BrowserImpl::SizeAllocate(const Rect& bounds) {
  SubwinView::SizeAllocate(bounds);
  Microsoft::WRL::ComPtr<IOleInPlaceObject> in_place;
  if (FAILED(browser_.As(&in_place)))
    return;
  RECT rc = { 0, 0, bounds.width(), bounds.height() };
  in_place->SetObjectRects(&rc, &rc);
}

bool BrowserImpl::HasFocus() const {
  return ::GetFocus() == browser_hwnd_;
}

void BrowserImpl::OnDestroy() {
  // The HWND of the window can be destroyed before the destructor is called.
  CleanupBrowserHWND();
}

void BrowserImpl::OnSetFocus(HWND hwnd) {
  // Still mark this control as focused.
  SubwinView::OnSetFocus(hwnd);
  SetMsgHandled(false);
  // But move the focus to the IE control.
  Microsoft::WRL::ComPtr<IOleInPlaceActiveObject> in_place_active;
  if (FAILED(browser_.As(&in_place_active)))
    return;
  in_place_active->OnFrameWindowActivate(TRUE);
  in_place_active->OnDocWindowActivate(TRUE);
}

LRESULT BrowserImpl::OnParentNotify(UINT msg, WPARAM w_param, LPARAM l_param) {
  if (w_param == WM_DESTROY) {
    // This is the only way to know when page calls window.close().
    CleanupBrowserHWND();
    auto* browser = static_cast<Browser*>(delegate());
    browser->on_close.Emit(browser);
  } else {
    SetMsgHandled(false);
  }
  return 0;
}

void BrowserImpl::ReceiveBrowserHWND() {
  // This function is called whenever navigation happens, stop once succeeded.
  if (browser_hwnd_)
    return;

  // Find the HWND of the actual IE control.
  Microsoft::WRL::ComPtr<IServiceProvider> service_provider;
  Microsoft::WRL::ComPtr<IOleWindow> ole_window;
  if (FAILED(browser_.As(&service_provider)) ||
      FAILED(service_provider->QueryService(SID_SShellBrowser,
                                            IID_PPV_ARGS(&ole_window))) ||
      FAILED(ole_window->GetWindow(&browser_hwnd_))) {
    PLOG(ERROR) << "Failed to get browser HWND";
    return;
  }
  if (browser_hwnd_)
    browser_hwnd_ = ::FindWindowEx(browser_hwnd_, nullptr,
                                   L"Shell DocObject View", nullptr);
  if (browser_hwnd_)
    browser_hwnd_ = ::FindWindowEx(browser_hwnd_, nullptr,
                                   L"Internet Explorer_Server", nullptr);

  // Hook window message proc on IE control.
  if (browser_hwnd_)
    browser_proc_ = SetWindowProc(browser_hwnd_, &BrowserWndProc);
}

void BrowserImpl::CleanupBrowserHWND() {
  if (browser_hwnd_)
    SetWindowProc(browser_hwnd_, browser_proc_);
}

// static
LRESULT BrowserImpl::BrowserWndProc(HWND hwnd,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param) {
  // Get the nu::Browser HWND.
  HWND nu_hwnd = ::GetParent(::GetParent(::GetParent(hwnd)));
  auto* self = static_cast<BrowserImpl*>(GetWindowUserData(nu_hwnd));
  DCHECK(self);
  // Interpret key shortcuts.
  switch (message) {
    case WM_KEYUP:
    case WM_KEYDOWN:
      // Ask if ViewImpl wants to handle the key.
      self->OnKeyEvent(message, w_param, l_param);
      if (self->IsMsgHandled())
        return true;
      // Then pass the key as accelerator to browser.
      Microsoft::WRL::ComPtr<IOleInPlaceActiveObject> in_place_active;
      if (FAILED(self->browser_.As(&in_place_active)))
        break;
      MSG msg = { hwnd, message, w_param, l_param };
      if (SUCCEEDED(in_place_active->TranslateAccelerator(&msg)))
        return true;
      break;
  }
  // Return to the original proc.
  return CallWindowProc(self->browser_proc_, hwnd, message, w_param, l_param);
}

///////////////////////////////////////////////////////////////////////////////
// Public Browser API implementation.

Browser::Browser() {
  TakeOverView(new BrowserImpl(this));
}

Browser::~Browser() {
}

void Browser::LoadURL(const std::string& url) {
  static_cast<BrowserImpl*>(GetNative())->LoadURL(base::UTF8ToUTF16(url));
}

void Browser::ExecuteJavaScript(const std::string& code,
                                const ExecutionCallback& callback) {
  auto* browser = static_cast<BrowserImpl*>(GetNative());
  base::string16 ret;
  bool success = browser->Eval(base::UTF8ToUTF16(code), &ret);
  if (callback)
    MessageLoop::PostTask(std::bind(callback, success, base::UTF16ToUTF8(ret)));
}

}  // namespace nu
