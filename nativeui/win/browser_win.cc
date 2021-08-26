// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser_win.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/state.h"
#include "nativeui/win/browser/browser_impl_ie.h"

#if defined(WEBVIEW2_SUPPORT)
#include "nativeui/win/webview2/browser_impl_webview2.h"
#endif

namespace nu {

///////////////////////////////////////////////////////////////////////////////
// BrowserHolder

BrowserHolder::BrowserHolder(Browser::Options options, Browser* delegate)
    : SubwinView(delegate) {
  set_focusable(true);
  // Initialize COM and OLE.
  State::GetCurrent()->InitializeCOM();

#if defined(WEBVIEW2_SUPPORT)
  if (options.webview2_support && State::GetCurrent()->InitWebView2Loader()) {
    BrowserImpl* impl = new BrowserImplWebview2(std::move(options), this);
    if (impl_)  // impl_ taken, which means creation failed immediately
      delete impl;
    else        // impl_ not taken, creation continues
      impl_.reset(impl);
  } else {
#endif
    impl_.reset(new BrowserImplIE(std::move(options), this));
    browser_created_ = true;
#if defined(WEBVIEW2_SUPPORT)
  }
#endif
}

BrowserHolder::~BrowserHolder() {
}

void BrowserHolder::ReportBrowserHWND(HWND hwnd) {
  browser_hwnd_ = hwnd;
}

#if defined(WEBVIEW2_SUPPORT)
void BrowserHolder::OnWebView2Completed(BrowserImpl* sender, bool success) {
  browser_created_ = true;
  // Note that the callback might be called syncronously by WebView2, so
  // we may still be in the constructor and the |impl_| may still be null.
  if (success) {
    // Make webview fill the window if it is added.
    sender->SetBounds(Rect(size_allocation().size()).ToRECT());
  } else {
    // Fall back to IE if WebView2 is not available.
    impl_.reset(new BrowserImplIE(std::move(sender->options()), this));
  }
  // Run pending loads.
  if (delegate()->pending_load()) {
    delegate()->pending_load()();
    delegate()->pending_load() = nullptr;
  }
}
#endif

void BrowserHolder::SizeAllocate(const Rect& bounds) {
  SubwinView::SizeAllocate(bounds);
  impl_->SetBounds({0, 0, bounds.width(), bounds.height()});
}

void BrowserHolder::SetFocus(bool focus) {
  if (focus) {
    impl_->Focus();
  } else if (window()) {
    window()->focus_manager()->RemoveFocus(this);
    // Focus on parent window if the browser has the focus.
    //
    // Note that we don't use WM_KILLFOCUS message as it would move focus to
    // parent HWND which is the holder, and it then would enter the code path
    // of moving focus out of browser with TAB in WebView2.
    //
    // Also note that we have to test if browser_hwnd has the focus before
    // focusing on window, since SetFocus(false) might be called when another
    // subwin control has already got focus via mouse clicking.
    if (browser_hwnd() && ::GetFocus() == browser_hwnd())
      window()->FocusWithoutEvent();
  }
}

bool BrowserHolder::HasFocus() const {
  return impl_->HasFocus();
}

bool BrowserHolder::OnMouseWheel(NativeEvent event) {
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

bool BrowserHolder::ProcessWindowMessage(HWND hwnd,
                                         UINT message,
                                         WPARAM w_param,
                                         LPARAM l_param,
                                         LRESULT* result) {
  if (browser_created_ && impl_) {
    if (impl_->ProcessWindowMessage(hwnd, message, w_param, l_param, result))
      return true;
    if (message == WM_MOVE || message == WM_MOVING)
      impl_->OnMove();
#if defined(WEBVIEW2_SUPPORT)
    if (message == WM_SETFOCUS && impl_->IsWebView2()) {
      // For WebView2, focus would be moved to holder when WebView2 moves the
      // focus out of itself, and we should move focus to parent then.
      if (window())
        window()->AdvanceFocus();
      return true;
    }
#endif
  }
  return SubwinView::ProcessWindowMessage(
      hwnd, message, w_param, l_param, result);
}


///////////////////////////////////////////////////////////////////////////////
// BrowserImpl

BrowserImpl::BrowserImpl(Browser::Options options, BrowserHolder* holder)
    : options_(std::move(options)), holder_(holder) {}

BrowserImpl::~BrowserImpl() = default;

bool BrowserImpl::IsWebView2() const {
  return false;
}

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

bool Browser::IsWebView2() const {
#if defined(WEBVIEW2_SUPPORT)
  auto* holder = static_cast<BrowserHolder*>(GetNative());
  return holder->impl()->IsWebView2();
#else
  return false;
#endif
}

void Browser::LoadURL(const std::string& url) {
  auto* holder = static_cast<BrowserHolder*>(GetNative());
  auto* browser = holder->impl();
#if defined(WEBVIEW2_SUPPORT)
  if (!holder->browser_created()) {
    scoped_refptr<Browser> ref(this);
    pending_load_ = [ref, url]() { ref->LoadURL(url); };
    return;
  }
#endif
  browser->LoadURL(base::UTF8ToWide(url));
}

void Browser::LoadHTML(const std::string& html, const std::string& base_url) {
  auto* holder = static_cast<BrowserHolder*>(GetNative());
  auto* browser = holder->impl();
#if defined(WEBVIEW2_SUPPORT)
  if (!holder->browser_created()) {
    scoped_refptr<Browser> ref(this);
    pending_load_ = [ref, html, base_url]() { ref->LoadHTML(html, base_url); };
    return;
  }
#endif
  browser->LoadHTML(base::UTF8ToWide(html), base::UTF8ToWide(base_url));
}

std::string Browser::GetURL() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return base::WideToUTF8(browser->GetURL());
}

std::string Browser::GetTitle() {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  return base::WideToUTF8(browser->GetTitle());
}

void Browser::SetUserAgent(const std::string& ua) {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->SetUserAgent(ua);
}

void Browser::ExecuteJavaScript(const std::string& code,
                                const ExecutionCallback& callback) {
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->ExecuteJavaScript(base::UTF8ToWide(code), callback);
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
  auto* browser = static_cast<BrowserHolder*>(GetNative())->impl();
  browser->UpdateBindings();
}

// static
bool Browser::RegisterProtocol(const std::string& scheme,
                               ProtocolHandler handler) {
  return BrowserImplIE::RegisterProtocol(base::UTF8ToWide(scheme),
                                         std::move(handler));
}

// static
void Browser::UnregisterProtocol(const std::string& scheme) {
  return BrowserImplIE::UnregisterProtocol(base::UTF8ToWide(scheme));
}

}  // namespace nu
