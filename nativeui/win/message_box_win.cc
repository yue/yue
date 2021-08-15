// Copyright 2020 Cheng Zhao. All rights reserved.
// Copyright 2013 GitHub, Inc.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/message_box.h"

#include <commctrl.h>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/win/window_win.h"
#include "nativeui/window.h"

namespace nu {

namespace {

// Small command ID values are already taken by Windows, we have to start from
// a large number to avoid conflicts with Windows.
const int kIDStart = 100;

}  // namespace

struct MessageBoxImpl : base::PlatformThread::Delegate {
  TASKDIALOGCONFIG config = {0};
  HWND hwnd = NULL;
  base::win::ScopedHICON icon;
  std::wstring title;
  std::wstring text;
  std::wstring informative_text;
  std::vector<std::wstring> button_titles;
  std::vector<TASKDIALOG_BUTTON> buttons;

  MessageBox* box;
  base::Lock lock;
  bool should_close = false;

  // Callback of the task dialog. Used for storing the hwnd of task dialog when
  // it is created.
  static HRESULT CALLBACK
  TaskDialogCallback(HWND hwnd, UINT msg, WPARAM, LPARAM, LONG_PTR data) {
    MessageBoxImpl* self = reinterpret_cast<MessageBoxImpl*>(data);
    if (msg == TDN_CREATED) {
      base::AutoLock lock(self->lock);
      // If the dialog is cancelled then close it directly.
      if (self->should_close) {
        self->should_close = false;
        ::PostMessage(hwnd, WM_CLOSE, 0, 0);
      } else {
        self->hwnd = hwnd;
      }
    } else if (msg == TDN_DESTROYED) {
      base::AutoLock lock(self->lock);
      self->hwnd = NULL;
    }
    return S_OK;
  }

 private:
  void ThreadMain() {
    BOOL flag = FALSE;
    int res = 0;
    ::TaskDialogIndirect(&config, &res, nullptr, &flag);
    MessageLoop::PostTask([=]() {
      if (res == 0 || res == IDCANCEL)
        box->OnClose();
      else
        box->OnClose(res - kIDStart);
    });
  }
};

MessageBox::MessageBox() {
  box_ = new MessageBoxImpl;
  box_->box = this;
  box_->config.cbSize = sizeof(box_->config);
  box_->config.hInstance = ::GetModuleHandle(NULL);
  box_->config.dwFlags =
      TDF_SIZE_TO_CONTENT |           // Show all content.
      TDF_ALLOW_DIALOG_CANCELLATION;  // Allow canceling the dialog.
  box_->config.pfCallback = &MessageBoxImpl::TaskDialogCallback;
  box_->config.lpCallbackData = reinterpret_cast<LONG_PTR>(box_);
  // Empty string would end up with "yue.exe", use space instead.
  box_->config.pszWindowTitle = L" ";
  // Reserve some space for buttons.
  box_->button_titles.reserve(4);
  box_->buttons.reserve(4);
}

MessageBox::~MessageBox() {
  delete box_;
}

int MessageBox::PlatformRun() {
  return PlatformRunForWindow(nullptr);
}

int MessageBox::PlatformRunForWindow(Window* window) {
  box_->config.hwndParent = window ? window->GetNative()->hwnd() : NULL;
  int res = cancel_response_;
  BOOL flag = FALSE;
  ::TaskDialogIndirect(&box_->config, &res, nullptr, &flag);
  return (res == 0 || res == IDCANCEL) ? cancel_response_ : res - kIDStart;
}

void MessageBox::PlatformShow() {
  PlatformShowForWindow(nullptr);
}

void MessageBox::PlatformShowForWindow(Window* window) {
  box_->config.hwndParent = window ? window->GetNative()->hwnd() : NULL;
  if (!base::PlatformThread::CreateNonJoinable(0, box_))
    OnClose();
}

void MessageBox::PlatformClose() {
  base::AutoLock lock(box_->lock);
  if (box_->hwnd)
    ::PostMessage(box_->hwnd, WM_CLOSE, 0, 0);
  else  // the dialog is not created yet
    box_->should_close = true;
}

void MessageBox::SetType(Type type) {
  switch (type) {
    case MessageBox::Type::None:
      box_->config.pszMainIcon = 0;
      break;
    case MessageBox::Type::Information:
      box_->config.pszMainIcon = TD_INFORMATION_ICON;
      break;
    case MessageBox::Type::Warning:
      box_->config.pszMainIcon = TD_WARNING_ICON;
      break;
    case MessageBox::Type::Error:
      box_->config.pszMainIcon = TD_ERROR_ICON;
      break;
  }
}

void MessageBox::SetTitle(const std::string& title) {
  box_->title = base::UTF8ToWide(title);
  box_->config.pszWindowTitle = box_->title.c_str();
}

void MessageBox::AddButton(const std::string& title, int response) {
  box_->button_titles.push_back(base::UTF8ToWide(title));
  box_->buttons.push_back({static_cast<int>(kIDStart + response)});
  // Must update all pointers in |buttons| since |titles| are changed.
  for (size_t i = 0; i < box_->button_titles.size(); ++i)
    box_->buttons[i].pszButtonText = box_->button_titles[i].c_str();
  box_->config.pButtons = &box_->buttons.front();
  box_->config.cButtons = box_->buttons.size();
}

void MessageBox::PlatformSetDefaultResponse() {
  box_->config.nDefaultButton = kIDStart + *default_response_;
}

void MessageBox::PlatformSetCancelResponse() {
}

void MessageBox::SetText(const std::string& text) {
  box_->text = base::UTF8ToWide(text);
  box_->config.pszMainInstruction = box_->text.c_str();
}

void MessageBox::SetInformativeText(const std::string& text) {
  box_->informative_text = base::UTF8ToWide(text);
  box_->config.pszContent = box_->informative_text.c_str();
}

void MessageBox::PlatformSetImage(Image* image) {
  SizeF size(image->GetNative()->GetWidth(), image->GetNative()->GetHeight());
  box_->icon = image->GetHICON(size);
  box_->config.dwFlags |= TDF_USE_HICON_MAIN;
  box_->config.hMainIcon = box_->icon.get();
}

}  // namespace nu
