// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/win/browser/browser_impl_ie.h"

#include <shlguid.h>

#include <map>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/message_loop.h"
#include "nativeui/win/browser/browser_protocol_factory.h"
#include "nativeui/win/browser/browser_util.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

// Stores the protocol factories.
std::map<std::wstring,
         Microsoft::WRL::ComPtr<BrowserProtocolFactory>> g_protocol_factories;

}  // namespace

// static
bool BrowserImplIE::RegisterProtocol(std::wstring scheme,
                                     Browser::ProtocolHandler handler) {
  Microsoft::WRL::ComPtr<IInternetSession> session;
  if (FAILED(::CoInternetGetSession(0, &session, 0)))
    return false;
  Microsoft::WRL::ComPtr<BrowserProtocolFactory> factory =
      new BrowserProtocolFactory(std::move(handler));
  g_protocol_factories[scheme] = factory;
  session->RegisterNameSpace(factory.Get(),
                             BrowserProtocolFactory::CLSID_BROWSER_PROTOCOL,
                             scheme.c_str(),
                             0, NULL, 0);
  return true;
}

// static
void BrowserImplIE::UnregisterProtocol(std::wstring scheme) {
  Microsoft::WRL::ComPtr<IInternetSession> session;
  if (FAILED(::CoInternetGetSession(0, &session, 0)))
    return;
  session->UnregisterNameSpace(g_protocol_factories[scheme].Get(),
                               scheme.c_str());
  g_protocol_factories.erase(scheme);
}

BrowserImplIE::BrowserImplIE(Browser::Options options, BrowserHolder* holder)
    : BrowserImpl(std::move(options), holder),
      external_sink_(new BrowserExternalSink(this)),
      ole_site_(new BrowserOleSite(this, external_sink_.Get())),
      event_sink_(new BrowserEventSink(this)),
      document_events_(new BrowserDocumentEvents(this)),
      weak_factory_(this) {
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

void BrowserImplIE::LoadURL(std::wstring str) {
  if (!browser_)
    return;
  html_moniker_.Reset();
  event_sink_->set_load_html(false);
  base::win::ScopedBstr url(str.c_str());
  browser_->Navigate(url.Get(), nullptr, nullptr, nullptr, nullptr);
}

void BrowserImplIE::LoadHTML(std::wstring str,
                             std::wstring base_url) {
  if (!browser_)
    return;
  html_moniker_ = new BrowserHTMLMoniker;
  html_moniker_->LoadHTML(str, base_url.empty() ? L"about:blank" : base_url);
  event_sink_->set_load_html(true);
  is_html_loaded_ = false;
  base::win::ScopedBstr url(L"about:blank");
  browser_->Navigate(url.Get(), nullptr, nullptr, nullptr, nullptr);
}

std::wstring BrowserImplIE::GetURL() {
  base::win::ScopedBstr url;
  browser_->get_LocationURL(url.Receive());
  return url.Length() == 0 ? L"about:blank" : url.Get();
}

std::wstring BrowserImplIE::GetTitle() {
  base::win::ScopedBstr title;
  if (document_)
    document_->get_title(title.Receive());
  if (!title.Get())
    return std::wstring();
  return std::wstring(title.Get());
}

void BrowserImplIE::SetUserAgent(const std::string& ua) {
  // Unfortunately we can only set global user agent.
  ::UrlMkSetSessionOption(URLMON_OPTION_USERAGENT_REFRESH, NULL, 0, 0);
  ::UrlMkSetSessionOption(URLMON_OPTION_USERAGENT,
                          (LPVOID)ua.c_str(), (WORD)ua.size(), 0);  // NOLINT
}

void BrowserImplIE::ExecuteJavaScript(
    std::wstring code,
    const Browser::ExecutionCallback& callback) {
  // IE executes the script syncronously but we need async behavior to keep
  // consistency with other platforms, so delay the task to next tick.
  //
  // Note that std::function does not support move-only types, so we have to
  // keep a copy of all arguments instead of using std::bind.
  auto ref = weak_factory_.GetWeakPtr();
  MessageLoop::PostTask([ref, code, callback]() {
    if (ref) {
      base::Value result;
      bool success = ref->Eval(code, &result);
      if (callback)
        callback(success, std::move(result));
    }
  });
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

void BrowserImplIE::Focus() {
  Microsoft::WRL::ComPtr<IOleInPlaceActiveObject> in_place_active;
  if (FAILED(browser_.As(&in_place_active)))
    return;
  in_place_active->OnFrameWindowActivate(TRUE);
  in_place_active->OnDocWindowActivate(TRUE);
}

bool BrowserImplIE::HasFocus() const {
  return ::GetFocus() == browser_hwnd();
}

bool BrowserImplIE::GetScript(Microsoft::WRL::ComPtr<IDispatchEx>* out) {
  if (!document_)
    return false;
  Microsoft::WRL::ComPtr<IDispatch> script_disp;
  return SUCCEEDED(document_->get_Script(&script_disp)) && script_disp &&
         SUCCEEDED(script_disp.As(out));
}

void BrowserImplIE::OnDestroy() {
  // The HWND of the window can be destroyed before the destructor is called.
  CleanupBrowserHWND();
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

bool BrowserImplIE::Eval(std::wstring code, base::Value* result) {
  Microsoft::WRL::ComPtr<IDispatchEx> script;
  if (!GetScript(&script))
    return false;
  base::win::ScopedVariant arg(code.c_str(), static_cast<UINT>(code.length()));
  base::win::ScopedVariant ret;
  if (!Invoke(script.Get(), L"eval", DISPATCH_METHOD, &ret, arg))
    return false;
  if (result)
    *result = VARIANTToValue(script.Get(), ret);
  return true;
}

void BrowserImplIE::ReceiveBrowserHWND() {
  // This function is called whenever navigation happens, stop once succeeded.
  if (browser_hwnd())
    return;

  // Find the HWND of the actual IE control.
  HWND browser_hwnd = NULL;
  Microsoft::WRL::ComPtr<IServiceProvider> service_provider;
  Microsoft::WRL::ComPtr<IOleWindow> ole_window;
  if (FAILED(browser_.As(&service_provider)) ||
      FAILED(service_provider->QueryService(SID_SShellBrowser,
                                            IID_PPV_ARGS(&ole_window))) ||
      FAILED(ole_window->GetWindow(&browser_hwnd))) {
    PLOG(ERROR) << "Failed to get browser HWND";
    return;
  }
  if (browser_hwnd)
    browser_hwnd = ::FindWindowEx(browser_hwnd, nullptr,
                                  L"Shell DocObject View", nullptr);
  if (browser_hwnd)
    browser_hwnd = ::FindWindowEx(browser_hwnd, nullptr,
                                  L"Internet Explorer_Server", nullptr);

  // Hook window message proc on IE control.
  if (browser_hwnd) {
    holder()->ReportBrowserHWND(browser_hwnd);
    browser_proc_ = SetWindowProc(browser_hwnd, &BrowserWndProc);
  }
}

void BrowserImplIE::CleanupBrowserHWND() {
  if (browser_hwnd())
    SetWindowProc(browser_hwnd(), browser_proc_);
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
  Eval(base::UTF8ToWide(browser->GetBindingScript()), nullptr);
}

// static
LRESULT BrowserImplIE::BrowserWndProc(HWND hwnd,
                                      UINT message,
                                      WPARAM w_param,
                                      LPARAM l_param) {
  // Get the nu::Browser HWND.
  HWND nu_hwnd = ::GetParent(::GetParent(::GetParent(hwnd)));
  auto* holder = static_cast<BrowserHolder*>(GetWindowUserData(nu_hwnd));
  if (!holder)  // could happen when dragging from Explorer to browser
    return false;
  auto* self = static_cast<BrowserImplIE*>(holder->impl());
  // Interpret key shortcuts.
  switch (message) {
    case WM_SETFOCUS:
      if (holder->window())
        holder->window()->focus_manager()->TakeFocus(holder);
      break;
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
