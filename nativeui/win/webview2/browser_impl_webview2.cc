// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/win/webview2/browser_impl_webview2.h"

#include <utility>

#include "base/base_paths.h"
#include "base/base_paths_win.h"
#include "base/file_version_info.h"
#include "base/path_service.h"

namespace nu {

namespace {

// Get the application name with order:
// 1. The product name specified in exe file.
// 2. The name of the exe file.
// 3. "Yue.WebView2"
base::string16 GetApplicationName() {
  base::string16 name;
  base::FilePath path;
  if (PathService::Get(base::FILE_EXE, &path)) {
    auto info = FileVersionInfo::CreateFileVersionInfo(path);
    if (info && !info->product_name().empty())
      name = info->product_name();
    else
      name = path.BaseName().RemoveExtension().value();
  }
  return name.empty() ? L"Yue.WebView2" : name;
}

// C:\Users\USER_NAME\AppData\Local\APPLICATION_NAME
base::FilePath GetUserDataDir() {
  base::FilePath path;
  if (!PathService::Get(base::DIR_LOCAL_APP_DATA, &path))
    PathService::Get(base::DIR_TEMP, &path);
  return path.Append(GetApplicationName());
}

}  // namespace

BrowserImplWebview2::BrowserImplWebview2(Browser::Options options,
                                         Browser* delegate)
    : BrowserImpl(std::move(options), delegate) {
  auto callback =
      Microsoft::WRL::Callback<
          ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
              this, &BrowserImplWebview2::OnEnvCreated);
  if (FAILED(::CreateCoreWebView2EnvironmentWithOptions(
                 nullptr, GetUserDataDir().value().c_str(), nullptr,
                 callback.Get()))) {
    return;
  }
}

BrowserImplWebview2::~BrowserImplWebview2() {
}

void BrowserImplWebview2::LoadURL(base::string16 str) {
  if (webview_)
    webview_->Navigate(str.c_str());
}

void BrowserImplWebview2::LoadHTML(base::string16 str,
                                   base::string16 base_url) {
}

base::string16 BrowserImplWebview2::GetURL() {
  return base::string16();
}

base::string16 BrowserImplWebview2::GetTitle() {
  return base::string16();
}

bool BrowserImplWebview2::Eval(base::string16 code, base::string16* result) {
  return false;
}

void BrowserImplWebview2::GoBack() {
}

bool BrowserImplWebview2::CanGoBack() const {
  return false;
}

void BrowserImplWebview2::GoForward() {
}

bool BrowserImplWebview2::CanGoForward() const {
  return false;
}

void BrowserImplWebview2::Reload() {
}

void BrowserImplWebview2::Stop() {
}

bool BrowserImplWebview2::IsLoading() const {
  return false;
}

void BrowserImplWebview2::SizeAllocate(const Rect& bounds) {
  SubwinView::SizeAllocate(bounds);
  if (controller_)
    controller_->put_Bounds({0, 0, bounds.width(), bounds.height()});
}

HRESULT BrowserImplWebview2::OnEnvCreated(HRESULT res,
                                          ICoreWebView2Environment* env) {
  if (FAILED(res))
    return res;
  auto callback =
      Microsoft::WRL::Callback<
          ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
              this, &BrowserImplWebview2::OnControllerCreated);
  if (FAILED(env->CreateCoreWebView2Controller(hwnd(), callback.Get())))
    return E_FAIL;
  env_ = env;
  return S_OK;
}

HRESULT BrowserImplWebview2::OnControllerCreated(
    HRESULT res, ICoreWebView2Controller* controller) {
  if (FAILED(res))
    return res;
  if (FAILED(controller->get_CoreWebView2(&webview_)))
    return E_FAIL;
  controller_ = controller;
  if (window())
    controller_->put_Bounds({0, 0,
                             size_allocation().width(),
                             size_allocation().height()});
  return S_OK;
}

}  // namespace nu
