// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_NOTIFICATIONS_COM_SERVER_MODULE_H_
#define NATIVEUI_WIN_NOTIFICATIONS_COM_SERVER_MODULE_H_

#include <guiddef.h>

#include <string>

#include "base/win/windows_types.h"

namespace nu {

// This class is used to host the NotificationActivator COM component and serve
// as the module for an out-of-proc COM server.
class ComServerModule {
 public:
  explicit ComServerModule(::GUID clsid);
  ~ComServerModule();

  ComServerModule& operator=(const ComServerModule&) = delete;
  ComServerModule(const ComServerModule&) = delete;

  // Registers the NotificationActivator COM object so other applications can
  // connect to it. Returns the registration status.
  HRESULT RegisterClassObjects();

  // Unregisters the NotificationActivator COM object. Returns the
  // unregistration status.
  HRESULT UnregisterClassObjects();

 private:
  void OnObjectCountZero();

  // The CLSID of NotificationActivator.
  ::GUID clsid_;

  // Identifiers of registered class objects. Used for unregistration.
  DWORD cookies_[1] = {0};
};

}  // namespace nu

#endif  // NATIVEUI_WIN_NOTIFICATIONS_COM_SERVER_MODULE_H_
