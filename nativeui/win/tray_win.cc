// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/tray_win.h"

#include <string>
#include <utility>

#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/menu.h"
#include "nativeui/state.h"
#include "nativeui/win/screen_win.h"

namespace nu {

TrayImpl::TrayImpl(Tray* delegate)
    : delegate_(delegate),
      host_(State::GetCurrent()->GetTrayHost()),
      icon_id_(host_->NextIconId()) {
  host_->Add(this);

  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_MESSAGE;
  icon_data.uCallbackMessage = TrayHost::kMessage;
  BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
  // This can happen if the explorer process isn't running when we try to
  // create the icon for some reason (for example, at startup).
  if (!result)
    LOG(WARNING) << "Unable to create tray icon";
}

TrayImpl::~TrayImpl() {
  Remove();
}

void TrayImpl::Remove() {
  if (!host_)
    return;
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  Shell_NotifyIcon(NIM_DELETE, &icon_data);

  host_->Remove(this);
  host_ = nullptr;
}

void TrayImpl::HandleClickEvent(UINT message) {
  if (message == WM_LBUTTONDOWN) {
    delegate_->on_click.Emit(delegate_);
  } else if (message == WM_RBUTTONUP && delegate_->GetMenu()) {
    // MSDN: To display a context menu for a notification icon, the current
    // window must be the foreground window before the application calls
    // TrackPopupMenu or TrackPopupMenuEx.
    ::SetForegroundWindow(State::GetCurrent()->GetSubwinHolder());
    delegate_->GetMenu()->Popup();
  }
}

void TrayImpl::ResetIcon() {
  if (!host_)
    return;
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  // Delete any previously existing icon.
  Shell_NotifyIcon(NIM_DELETE, &icon_data);
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_MESSAGE;
  icon_data.uCallbackMessage = TrayHost::kMessage;
  icon_data.hIcon = icon_.get();
  // If we have an image, then set the NIF_ICON flag, which tells
  // Shell_NotifyIcon() to set the image for the status icon it creates.
  if (icon_data.hIcon)
    icon_data.uFlags |= NIF_ICON;
  // Re-add our icon.
  BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
  if (!result)
    LOG(WARNING) << "Unable to re-create status tray icon.";
}

Rect TrayImpl::GetBounds() const {
  if (!host_)
    return Rect();
  NOTIFYICONIDENTIFIER identifier = {0};
  identifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
  identifier.hWnd = hwnd();
  identifier.uID = icon_id_;

  RECT rect = {0};
  Shell_NotifyIconGetRect(&identifier, &rect);
  return Rect(rect);
}

void TrayImpl::SetImage(Image* icon) {
  if (!host_)
    return;
  // Convert image to bitmap icon.
  int width = ::GetSystemMetrics(SM_CXSMICON);
  icon_ = icon->GetHICON(SizeF(width, width));

  // Create the icon.
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_ICON;
  icon_data.hIcon = icon_.get();
  BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
  if (!result)
    LOG(WARNING) << "Error setting tray image";
}

void TrayImpl::InitIconData(NOTIFYICONDATA* icon_data) {
  memset(icon_data, 0, sizeof(NOTIFYICONDATA));
  icon_data->cbSize = sizeof(NOTIFYICONDATA);
  icon_data->hWnd = hwnd();
  icon_data->uID = icon_id_;
}

///////////////////////////////////////////////////////////////////////////////
// Public Tray API implementation.

Tray::Tray(scoped_refptr<Image> icon) : tray_(new TrayImpl(this)) {
  tray_->SetImage(icon.get());
}

Tray::~Tray() {
  delete tray_;
}

void Tray::Remove() {
  tray_->Remove();
}

RectF Tray::GetBounds() const {
  return ScaleRect(RectF(tray_->GetBounds()),
                         1.0f / GetScaleFactorForHWND(tray_->hwnd()));
}

void Tray::SetImage(scoped_refptr<Image> icon) {
  tray_->SetImage(icon.get());
}

void Tray::SetMenu(scoped_refptr<Menu> menu) {
  menu_ = std::move(menu);
}

}  // namespace nu
