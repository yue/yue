// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_NOTIFICATIONS_WINRT_UTIL_H_
#define NATIVEUI_WIN_NOTIFICATIONS_WINRT_UTIL_H_

#include <windows.ui.notifications.h>
#include <wrl.h>

#include "base/win/core_winrt_util.h"
#include "base/win/scoped_hstring.h"

namespace mswr = Microsoft::WRL;
namespace winfoundtn = ABI::Windows::Foundation;
namespace winui = ABI::Windows::UI;
namespace winxml = ABI::Windows::Data::Xml;

using base::win::ScopedHString;
using winui::Notifications::IToastNotification;
using winui::Notifications::INotificationData;

namespace nu {

// Templated wrapper for winfoundtn::GetActivationFactory().
template <unsigned int size>
HRESULT CreateActivationFactory(wchar_t const (&class_name)[size],
                                const IID& iid,
                                void** factory) {
  ScopedHString ref_class_name =
      ScopedHString::Create(base::WStringPiece(class_name, size - 1));
  return base::win::RoGetActivationFactory(ref_class_name.get(), iid, factory);
}

}  // namespace nu

#endif  // NATIVEUI_WIN_NOTIFICATIONS_WINRT_UTIL_H_
