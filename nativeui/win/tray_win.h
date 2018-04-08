// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_TRAY_WIN_H_
#define NATIVEUI_WIN_TRAY_WIN_H_

#include "base/win/scoped_gdi_object.h"
#include "nativeui/tray.h"

namespace nu {

class TrayHost;

class TrayImpl {
 public:
  explicit TrayImpl(Tray* delegate);
  ~TrayImpl();

  void HandleClickEvent(UINT message);
  void ResetIcon();
  void SetImage(Image* icon);

  UINT icon_id() const { return icon_id_; }

 private:
  void InitIconData(NOTIFYICONDATA* icon_data);

  Tray* delegate_;

  // The host that manages the tray.
  TrayHost* host_;

  // The unique ID corresponding to this icon.
  UINT icon_id_;

  // The currently-displayed icon for the window.
  base::win::ScopedHICON icon_;

  DISALLOW_COPY_AND_ASSIGN(TrayImpl);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_TRAY_WIN_H_
