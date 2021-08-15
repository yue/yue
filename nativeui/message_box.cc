// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_box.h"

#include <utility>

#include "base/logging.h"
#include "nativeui/gfx/image.h"

namespace nu {

int MessageBox::Run() {
  if (is_showing_) {
    LOG(ERROR) << "MessageBox is already showing";
    return cancel_response_;
  }
  is_showing_ = true;
  AddRef();
  int res = PlatformRun();
#if !defined(OS_LINUX)
  OnClose(res);
#endif
  return res;
}

int MessageBox::RunForWindow(Window* window) {
  if (!window)
    return Run();
  if (is_showing_) {
    LOG(ERROR) << "MessageBox is already showing";
    return cancel_response_;
  }
  is_showing_ = true;
  AddRef();
  int res = PlatformRunForWindow(window);
#if !defined(OS_LINUX)
  OnClose(res);
#endif
  return res;
}

#if defined(OS_LINUX) || defined(OS_WIN)
void MessageBox::Show() {
  if (is_showing_) {
    LOG(ERROR) << "MessageBox is already showing";
    return;
  }
  is_showing_ = true;
  AddRef();
  PlatformShow();
}
#endif

void MessageBox::ShowForWindow(Window* window) {
#if defined(OS_LINUX) || defined(OS_WIN)
  if (!window) {
    Show();
    return;
  }
#else
  CHECK(window);
#endif
  if (is_showing_) {
    LOG(ERROR) << "MessageBox is already showing";
    return;
  }
  is_showing_ = true;
  AddRef();
  PlatformShowForWindow(window);
}

void MessageBox::Close() {
  if (!is_showing_) {
    LOG(ERROR) << "MessageBox is not showing";
    return;
  }
  PlatformClose();
}

void MessageBox::SetDefaultResponse(int response) {
  default_response_.emplace(response);
  PlatformSetDefaultResponse();
}

void MessageBox::SetCancelResponse(int response) {
  cancel_response_ = response;
  PlatformSetCancelResponse();
}

void MessageBox::SetImage(scoped_refptr<Image> image) {
  PlatformSetImage(image.get());
  image_ = std::move(image);
}

void MessageBox::OnClose(absl::optional<int> response) {
  is_showing_ = false;
  on_response.Emit(this, response ? *response : cancel_response_);
  Release();
}

}  // namespace nu
