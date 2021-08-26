// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_protocol_factory.h"

#include <shlwapi.h>
#include <wrl.h>

#include <utility>

#include "nativeui/win/browser/browser_protocol.h"

namespace nu {

namespace {

ULONG g_server_locks = 0;

}  // namespace

// static
const GUID BrowserProtocolFactory::CLSID_BROWSER_PROTOCOL =
    { 0xa70680fd, 0x7612, 0x4d2e,
      { 0x85, 0x35, 0x23, 0x32, 0xfb, 0x91, 0x73, 0x96 } };

BrowserProtocolFactory::BrowserProtocolFactory(
    Browser::ProtocolHandler handler)
    : ref_(0),
      handler_(std::move(handler)) {
}

BrowserProtocolFactory::~BrowserProtocolFactory() {
}

STDMETHODIMP BrowserProtocolFactory::QueryInterface(REFIID riid,
                                                    void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserProtocolFactory, IClassFactory),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserProtocolFactory::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserProtocolFactory::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

HRESULT STDMETHODCALLTYPE BrowserProtocolFactory::CreateInstance(
    IUnknown *pUnkOuter, REFIID riid, void **ppvObject) {
  if (pUnkOuter != nullptr)
    return CLASS_E_NOAGGREGATION;
  Microsoft::WRL::ComPtr<BrowserProtocol> protocol =
      new BrowserProtocol(handler_);
  return protocol->QueryInterface(riid, ppvObject);
}

HRESULT STDMETHODCALLTYPE BrowserProtocolFactory::LockServer(BOOL fLock) {
  if (fLock)
    InterlockedIncrement(&g_server_locks);
  else
    InterlockedDecrement(&g_server_locks);
  return S_OK;
}

}  // namespace nu
