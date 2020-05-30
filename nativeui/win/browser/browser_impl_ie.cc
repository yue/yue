// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/win/browser/browser_impl_ie.h"

#include <shlguid.h>

#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_bstr.h"
#include "base/win/scoped_variant.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/win/browser/browser_util.h"
#include "nativeui/win/util/dispatch_invoke.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

// Convert a VARIANT to JSON string.
bool VARIANTToJSON(IDispatchEx* script,
                   const base::win::ScopedVariant& value,
                   base::string16* result) {
  // Can't pass empty VARIANT to IE.
  if (value.type() == VT_EMPTY) {
    *result = L"undefined";
    return true;
  }
  // Find the javascript JSON object.
  base::win::ScopedVariant json_var;
  if (!Invoke(script, L"JSON", DISPATCH_PROPERTYGET, &json_var))
    return false;
  Microsoft::WRL::ComPtr<IDispatch> json_disp =
      static_cast<IDispatch*>(json_var.ptr()->pdispVal);
  Microsoft::WRL::ComPtr<IDispatchEx> json_obj;
  if (FAILED(json_disp.As(&json_obj)) || !json_obj)
    return false;
  // Invoke the JSON.stringify method.
  base::win::ScopedVariant str;
  if (!Invoke(json_obj.Get(), L"stringify", DISPATCH_METHOD, &str, value))
    return false;
  *result = str.ptr()->bstrVal;
  return true;
}

}  // namespace

BrowserImplIE::BrowserImplIE(Browser::Options options,
                             BrowserHolder* holder,
                             Browser* delegate)
    : BrowserImpl(std::move(options), holder, delegate),
      external_sink_(new BrowserExternalSink(delegate)),
      ole_site_(new BrowserOleSite(this, external_sink_.Get())),
      event_sink_(new BrowserEventSink(this)),
      document_events_(new BrowserDocumentEvents(this)) {
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
  ole_object->DoVerb(OLEIVERB_SHOW, nullptr, ole_site_.Get(), -1, hwnd(), &rc);
}

BrowserImplIE::~BrowserImplIE() {
  CleanupBrowserHWND();
  // IE does not automatically cleanup.
  Microsoft::WRL::ComPtr<IOleObject> ole_object;
  if (FAILED(browser_.As(&ole_object)))
    return;
  ole_object->DoVerb(OLEIVERB_HIDE, nullptr, ole_site_.Get(), -1,
                     hwnd(), nullptr);
  ole_object->Close(OLECLOSE_NOSAVE);
  ole_object->SetClientSite(nullptr);
}

void BrowserImplIE::LoadURL(base::string16 str) {
  if (!browser_)
    return;
  html_moniker_.Reset();
  event_sink_->set_load_html(false);
  base::win::ScopedBstr url(str.c_str());
  browser_->Navigate(url, nullptr, nullptr, nullptr, nullptr);
}

void BrowserImplIE::LoadHTML(base::string16 str,
                             base::string16 base_url) {
  if (!browser_)
    return;
  html_moniker_ = new BrowserHTMLMoniker;
  html_moniker_->LoadHTML(str, base_url.empty() ? L"about:blank" : base_url);
  event_sink_->set_load_html(true);
  is_html_loaded_ = false;
  base::win::ScopedBstr url(L"about:blank");
  browser_->Navigate(url, nullptr, nullptr, nullptr, nullptr);
}

base::string16 BrowserImplIE::GetURL() {
  base::win::ScopedBstr url;
  browser_->get_LocationURL(url.Receive());
  return url.Length() == 0 ? L"about:blank" : url;
}

base::string16 BrowserImplIE::GetTitle() {
  base::win::ScopedBstr title;
  if (document_)
    document_->get_title(title.Receive());
  if (!title)
    return base::string16();
  return base::string16(title);
}

bool BrowserImplIE::Eval(base::string16 code, base::string16* result) {
  if (!document_)
    return false;
  Microsoft::WRL::ComPtr<IDispatch> script_disp;
  if (FAILED(document_->get_Script(&script_disp)) || !script_disp)
    return false;
  Microsoft::WRL::ComPtr<IDispatchEx> script;
  if (FAILED(script_disp.As(&script)))
    return false;
  base::win::ScopedVariant arg(code.c_str(), static_cast<UINT>(code.length()));
  base::win::ScopedVariant ret;
  if (!Invoke(script.Get(), L"eval", DISPATCH_METHOD, &ret, arg))
    return false;
  if (result && !VARIANTToJSON(script.Get(), ret, result))
    return false;
  return true;
}

void BrowserImplIE::GoBack() {
  if (browser_)
    browser_->GoBack();
}

bool BrowserImplIE::CanGoBack() const {
  return can_go_back_;
}

void BrowserImplIE::GoForward() {
  if (browser_)
    browser_->GoForward();
}

bool BrowserImplIE::CanGoForward() const {
  return can_go_forward_;
}

void BrowserImplIE::Reload() {
  if (browser_)
    browser_->Refresh();
}

void BrowserImplIE::Stop() {
  if (browser_)
    browser_->Stop();
}

bool BrowserImplIE::IsLoading() const {
  VARIANT_BOOL loading = false;
  if (browser_)
    browser_->get_Busy(&loading);
  return loading;
}

void BrowserImplIE::SetBounds(RECT rect) {
  Microsoft::WRL::ComPtr<IOleInPlaceObject> in_place;
  if (FAILED(browser_.As(&in_place)))
    return;
  in_place->SetObjectRects(&rect, &rect);
}

bool BrowserImplIE::HasFocus() const {
  return ::GetFocus() == browser_hwnd_;
}

bool BrowserImplIE::OnMouseWheel(NativeEvent event) {
  // For WM_MOUSEWHEEL received from the parent window, forward it to the
  // browser hwnd.
  if (!browser_hwnd_ || !window())
    return false;
  POINT p = { CR_GET_X_LPARAM(event->l_param),
              CR_GET_Y_LPARAM(event->l_param) };
  ::ClientToScreen(window()->hwnd(), &p);
  ::SendMessage(browser_hwnd_, event->message, event->w_param,
                MAKELPARAM(p.x, p.y));
  return true;
}

void BrowserImplIE::OnDestroy() {
  // The HWND of the window can be destroyed before the destructor is called.
  CleanupBrowserHWND();
}

void BrowserImplIE::OnSetFocus(HWND hwnd) {
  // Still mark this control as focused.
  holder()->OnSetFocus(hwnd);
  SetMsgHandled(false);
  // But move the focus to the IE control.
  Microsoft::WRL::ComPtr<IOleInPlaceActiveObject> in_place_active;
  if (FAILED(browser_.As(&in_place_active)))
    return;
  in_place_active->OnFrameWindowActivate(TRUE);
  in_place_active->OnDocWindowActivate(TRUE);
}

LRESULT BrowserImplIE::OnParentNotify(UINT msg, WPARAM w_param, LPARAM) {
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

void BrowserImplIE::ReceiveBrowserHWND() {
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

void BrowserImplIE::CleanupBrowserHWND() {
  if (browser_hwnd_)
    SetWindowProc(browser_hwnd_, browser_proc_);
}

void BrowserImplIE::OnDocumentReady() {
  // Get and cache the document object.
  Microsoft::WRL::ComPtr<IDispatch> doc2_disp;
  if (FAILED(browser_->get_Document(&doc2_disp)) || !doc2_disp ||
      FAILED(doc2_disp.As(&document_))) {
    LOG(ERROR) << "Failed to get document";
    return;
  }
  // Handling the LoadHTML request.
  if (html_moniker_ && !is_html_loaded_) {
    is_html_loaded_ = true;
    Microsoft::WRL::ComPtr<IPersistMoniker> moniker;
    if (FAILED(document_.As(&moniker)) ||
        FAILED(moniker->Load(TRUE, html_moniker_.Get(), nullptr, STGM_READ))) {
      LOG(ERROR) << "Failed to load HTML content";
    }
    return;
  }
  // Listen to events of document.
  Microsoft::WRL::ComPtr<IConnectionPointContainer> cpc;
  Microsoft::WRL::ComPtr<IConnectionPoint> cp;
  DWORD cookie;
  if (FAILED(document_.As(&cpc)) ||
      FAILED(cpc->FindConnectionPoint(DIID_HTMLDocumentEvents2, &cp)) ||
      FAILED(cp->Advise(document_events_.Get(), &cookie))) {
    PLOG(ERROR) << "Failed install set document events";
    return;
  }
  // Add bindings to the document.
  auto* browser = static_cast<Browser*>(delegate());
  Eval(base::UTF8ToUTF16(browser->GetBindingScript()), nullptr);
}

// static
LRESULT BrowserImplIE::BrowserWndProc(HWND hwnd,
                                      UINT message,
                                      WPARAM w_param,
                                      LPARAM l_param) {
  // Get the nu::Browser HWND.
  HWND nu_hwnd = ::GetParent(::GetParent(::GetParent(hwnd)));
  auto* holder = static_cast<BrowserHolder*>(GetWindowUserData(nu_hwnd));
  DCHECK(holder);
  auto* self = static_cast<BrowserImplIE*>(holder->impl());
  // Interpret key shortcuts.
  switch (message) {
    case WM_KEYUP:
    case WM_KEYDOWN:
      // Ask if ViewImpl wants to handle the key.
      holder->OnKeyEvent(message, w_param, l_param);
      if (holder->IsMsgHandled())
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

}  // namespace nu
