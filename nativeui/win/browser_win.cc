// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser_win.h"

#include <wrl.h>

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "base/json/json_reader.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/message_loop.h"
#include "nativeui/state.h"
#include "nativeui/win/browser/browser_impl_ie.h"
#include "nativeui/win/browser/browser_protocol_factory.h"

#if defined(WEBVIEW2_SUPPORT)
#include "nativeui/win/webview2/browser_impl_webview2.h"
#endif

namespace nu {

namespace {

// Stores the protocol factories.
std::map<base::string16,
         Microsoft::WRL::ComPtr<BrowserProtocolFactory>> g_protocol_factories;

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// BrowserHolder

BrowserHolder::BrowserHolder(Browser::Options options, Browser* delegate)
    : SubwinView(delegate) {
  set_focusable(true);
  // Initialize COM and OLE.
  State::GetCurrent()->InitializeCOM();

#if defined(WEBVIEW2_SUPPORT)
  if (options.webview2_support && State::GetCurrent()->InitWebView2Loader())
    impl_.reset(new BrowserImplWebview2(std::move(options), this, delegate));
  else
#endif
    impl_.reset(new BrowserImplIE(std::move(options), this, delegate));
}

BrowserHolder::~BrowserHolder() {
}

#if defined(WEBVIEW2_SUPPORT)
void BrowserHolder::OnWebView2Completed(bool success) {
  if (success) {
    // Make webview fill the window if it is added.
    impl_->SetBounds(Rect(size_allocation().size()).ToRECT());
  } else {
    // Fall back to IE if WebView2 is not available.
    impl_.reset(new BrowserImplIE(std::move(impl_->options()), this,
                                  impl_->delegate()));
  }
}
#endif

void BrowserHolder::SizeAllocate(const Rect& bounds) {
  SubwinView::SizeAllocate(bounds);
  impl_->SetBounds({0, 0, bounds.width(), bounds.height()});
}

bool BrowserHolder::HasFocus() const {
  return impl_->HasFocus();
}

bool BrowserHolder::OnMouseWheel(NativeEvent event) {
  return impl_->OnMouseWheel(event);
}

LRESULT BrowserHolder::OnMouseWheelFromSelf(
    UINT message, WPARAM w_param, LPARAM l_param) {
  // We might receive WM_MOUSEWHEEL in the shell hwnd when scrolled to edges,
  // do not pass the event to SubwinView otherwise we will have stack overflow.
  if (window()) {
    // Do nothing if the event happened inside the view.
    POINT p = { CR_GET_X_LPARAM(l_param), CR_GET_Y_LPARAM(l_param) };
    ::ScreenToClient(window()->hwnd(), &p);
    if (size_allocation().Contains(Point(p)))
      return 0;
  }
  return ::DefWindowProc(hwnd(), message, w_param, l_param);
}

bool BrowserHolder::ProcessWindowMessage(HWND window,
                                         UINT message,
                                         WPARAM w_param,
                                         LPARAM l_param,
                                         LRESULT* result) {
  if (impl_ &&
      impl_->ProcessWindowMessage(window, message, w_param, l_param, result)) {
    return true;
  }
  return SubwinView::ProcessWindowMessage(
      window, message, w_param, l_param, result);
}


///////////////////////////////////////////////////////////////////////////////
// BrowserImpl

BrowserImpl::BrowserImpl(Browser::Options options,
                         BrowserHolder* holder,
                         Browser* delegate)
    : options_(std::move(options)), holder_(holder), delegate_(delegate) {}

BrowserImpl::~BrowserImpl() = default;

bool BrowserImpl::ProcessWindowMessage(HWND window,
                                       UINT message,
                                       WPARAM w_param,
                                       LPARAM l_param,
                                       LRESULT* result) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Public Browser API implementation.

void Browser::PlatformInit(Options options) {
  TakeOverView(new BrowserHolder(options, this));
}

void Browser::PlatformDestroy() {
}

void Browser::LoadURL(const std::string& url) {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->LoadURL(base::UTF8ToUTF16(url));
}

void Browser::LoadHTML(const std::string& html, const std::string& base_url) {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->LoadHTML(base::UTF8ToUTF16(html), base::UTF8ToUTF16(base_url));
}

std::string Browser::GetURL() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return base::UTF16ToUTF8(browser->GetURL());
}

std::string Browser::GetTitle() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return base::UTF16ToUTF8(browser->GetTitle());
}

void Browser::SetUserAgent(const std::string& ua) {
  // Unfortunately we can only set global user agent.
  ::UrlMkSetSessionOption(URLMON_OPTION_USERAGENT_REFRESH, NULL, 0, 0);
  ::UrlMkSetSessionOption(URLMON_OPTION_USERAGENT,
                          (LPVOID)ua.c_str(), (WORD)ua.size(), 0);  // NOLINT
}

void Browser::ExecuteJavaScript(const std::string& code,
                                const ExecutionCallback& callback) {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  base::string16 json;
  bool success = browser->Eval(base::UTF8ToUTF16(code),
                               callback ? &json : nullptr);
  if (callback) {
    std::string json_str = base::UTF16ToUTF8(json);
    MessageLoop::PostTask([=]() {
      std::unique_ptr<base::Value> pv = base::JSONReader::Read(json_str);
      callback(success, pv ? std::move(*pv) : base::Value());
    });
  }
}

void Browser::GoBack() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->GoBack();
}

bool Browser::CanGoBack() const {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return browser->CanGoBack();
}

void Browser::GoForward() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->GoForward();
}

bool Browser::CanGoForward() const {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return browser->CanGoForward();
}

void Browser::Reload() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->Reload();
}

void Browser::Stop() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->Stop();
}

bool Browser::IsLoading() const {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return browser->IsLoading();
}

void Browser::PlatformUpdateBindings() {
}

// static
bool Browser::RegisterProtocol(const std::string& scheme,
                               const ProtocolHandler& handler) {
  Microsoft::WRL::ComPtr<IInternetSession> session;
  if (FAILED(::CoInternetGetSession(0, &session, 0)))
    return false;
  Microsoft::WRL::ComPtr<BrowserProtocolFactory> factory =
      new BrowserProtocolFactory(handler);
  base::string16 name = base::UTF8ToUTF16(scheme);
  g_protocol_factories[name] = factory;
  session->RegisterNameSpace(factory.Get(),
                             BrowserProtocolFactory::CLSID_BROWSER_PROTOCOL,
                             name.c_str(),
                             0, NULL, 0);
  return true;
}

// static
void Browser::UnregisterProtocol(const std::string& scheme) {
  Microsoft::WRL::ComPtr<IInternetSession> session;
  if (FAILED(::CoInternetGetSession(0, &session, 0)))
    return;
  base::string16 name = base::UTF8ToUTF16(scheme);
  session->UnregisterNameSpace(g_protocol_factories[name].Get(), name.c_str());
  g_protocol_factories.erase(name);
}

}  // namespace nu
