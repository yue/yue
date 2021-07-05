// Copyright 2021 Cheng Zhao. All rights reserved.
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

// This macro is used in <wrl/module.h>. Since only the COM functionality is
// used here (while WinRT isn't being used), define this macro to optimize
// compilation of <wrl/module.h> for COM-only.
#ifndef __WRL_CLASSIC_COM_STRICT__
#define __WRL_CLASSIC_COM_STRICT__
#endif  // __WRL_CLASSIC_COM_STRICT__

#include "nativeui/win/notifications/com_server_module.h"

#include <objbase.h>
#include <wrl/module.h>

#include <utility>

#include "base/logging.h"
#include "nativeui/win/notifications/notification_activator.h"
#include "nativeui/win/notifications/winrt_util.h"

namespace nu {

ComServerModule::ComServerModule(::GUID clsid)
    : clsid_(std::move(clsid)) {
}

ComServerModule::~ComServerModule() = default;

HRESULT ComServerModule::RegisterClassObjects() {
  // Create an out-of-proc COM module with caching disabled. The supplied
  // method is invoked when the last instance object of the module is released.
  auto& module = mswr::Module<mswr::OutOfProcDisableCaching>::Create(
      this, &ComServerModule::OnObjectCountZero);

  // Usually COM module classes statically define their CLSID at compile time
  // through the use of various macros, and WRL::Module internals takes care of
  // creating the class objects and registering them. However, we need to
  // register the same object with different CLSIDs depending on a runtime
  // setting, so we handle that logic here.

  mswr::ComPtr<IUnknown> factory;
  unsigned int flags = mswr::ModuleType::OutOfProcDisableCaching;

  HRESULT hr = mswr::Details::CreateClassFactory<
      mswr::SimpleClassFactory<NotificationActivator>>(
      &flags, nullptr, __uuidof(IClassFactory), &factory);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Factory creation failed " << std::hex << hr;
    return hr;
  }

  mswr::ComPtr<IClassFactory> class_factory;
  hr = factory.As(&class_factory);
  if (FAILED(hr)) {
    DLOG(ERROR) << "IClassFactory object creation failed " << std::hex << hr;
    return hr;
  }

  // All pointers in this array are unowned. Do not release them.
  IClassFactory* class_factories[] = {class_factory.Get()};

  hr = module.RegisterCOMObject(nullptr, &clsid_, class_factories, cookies_,
                                std::extent<decltype(cookies_)>());
  if (FAILED(hr)) {
    DLOG(ERROR) << "NotificationActivator registration failed "
                << std::hex << hr;
  }

  // By default the module's reference count will reduce to 0 when first
  // notification is received, and then shutdown.
  // As we still want to receive more notifications in this process, increase
  // the reference count so it never destroys automatically.
  hr = module.IncrementObjectCount();
  return hr;
}

HRESULT ComServerModule::UnregisterClassObjects() {
  auto& module = mswr::Module<mswr::OutOfProcDisableCaching>::GetModule();
  module.DecrementObjectCount();
  HRESULT hr = module.UnregisterCOMObject(nullptr, cookies_,
                                          std::extent<decltype(cookies_)>());
  if (FAILED(hr)) {
    DLOG(ERROR) << "NotificationActivator unregistration failed "
                << std::hex << hr;
  }
  return hr;
}

void ComServerModule::OnObjectCountZero() {
}

}  // namespace nu
