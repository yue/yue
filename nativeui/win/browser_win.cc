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

namespace nu {

namespace {

// Stores the protocol factories.
std::map<base::string16,
         Microsoft::WRL::ComPtr<BrowserProtocolFactory>> g_protocol_factories;

}  // namespace

BrowserImpl::BrowserImpl(Browser::Options options, Browser* delegate)
    : SubwinView(delegate),
      options_(std::move(options)) {
  set_focusable(true);
  // Initialize COM and OLE.
  State::GetCurrent()->InitializeCOM();
}

void Browser::PlatformInit(const Options& options) {
  TakeOverView(new BrowserImplIE(options, this));
}

void Browser::PlatformDestroy() {
}

void Browser::LoadURL(const std::string& url) {
  static_cast<BrowserImpl*>(GetNative())->LoadURL(base::UTF8ToUTF16(url));
}

void Browser::LoadHTML(const std::string& html, const std::string& base_url) {
  auto* browser = static_cast<BrowserImpl*>(GetNative());
  browser->LoadHTML(base::UTF8ToUTF16(html), base::UTF8ToUTF16(base_url));
}

std::string Browser::GetURL() {
  auto* browser = static_cast<BrowserImpl*>(GetNative());
  return base::UTF16ToUTF8(browser->GetURL());
}

std::string Browser::GetTitle() {
  auto* browser = static_cast<BrowserImpl*>(GetNative());
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
  auto* browser = static_cast<BrowserImpl*>(GetNative());
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
  static_cast<BrowserImpl*>(GetNative())->GoBack();
}

bool Browser::CanGoBack() const {
  return static_cast<BrowserImpl*>(GetNative())->CanGoBack();
}

void Browser::GoForward() {
  static_cast<BrowserImpl*>(GetNative())->GoForward();
}

bool Browser::CanGoForward() const {
  return static_cast<BrowserImpl*>(GetNative())->CanGoForward();
}

void Browser::Reload() {
  static_cast<BrowserImpl*>(GetNative())->Reload();
}

void Browser::Stop() {
  static_cast<BrowserImpl*>(GetNative())->Stop();
}

bool Browser::IsLoading() const {
  return static_cast<BrowserImpl*>(GetNative())->IsLoading();
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
