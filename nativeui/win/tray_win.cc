// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/tray_win.h"

#include <string>

#include "nativeui/gfx/canvas.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/menu.h"
#include "nativeui/state.h"
#include "nativeui/win/util/tray_host.h"

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
  host_->Remove(this);

  // Remove our icon.
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  Shell_NotifyIcon(NIM_DELETE, &icon_data);
}

void TrayImpl::HandleClickEvent(UINT message) {
  if (message == WM_LBUTTONDOWN)
    delegate_->on_click.Emit(delegate_);
  else if ((message == WM_RBUTTONDOWN || message == WM_CONTEXTMENU) &&
           delegate_->GetMenu())
    delegate_->GetMenu()->Popup();
}

void TrayImpl::ResetIcon() {
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

void TrayImpl::SetImage(Image* icon) {
  // Convert image to bitmap icon.
  int width = ::GetSystemMetrics(SM_CXSMICON);
  scoped_refptr<Canvas> canvas = new Canvas(SizeF(width, width));
  canvas->GetPainter()->DrawImage(icon, RectF(0, 0, width, width));
  if (canvas->GetBitmap()->GetGdiplusBitmap()->GetHICON(icon_.receive()) !=
          Gdiplus::Ok)
    LOG(WARNING) << "Error converting image to HICON";

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
  icon_data->hWnd = host_->hwnd();
  icon_data->uID = icon_id_;
}

///////////////////////////////////////////////////////////////////////////////
// Public Tray API implementation.

Tray::Tray(Image* icon) : tray_(new TrayImpl(this)) {
  tray_->SetImage(icon);
}

Tray::~Tray() {
  delete tray_;
}

void Tray::SetTitle(const std::string& title) {
}

void Tray::SetImage(Image* icon) {
  tray_->SetImage(icon);
}

void Tray::SetMenu(Menu* menu) {
  menu_ = menu;
}

}  // namespace nu
