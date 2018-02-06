// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_BROWSER_PROTOCOL_FACTORY_H_
#define NATIVEUI_WIN_BROWSER_BROWSER_PROTOCOL_FACTORY_H_

#include <ole2.h>

#include "nativeui/browser.h"

namespace nu {

class BrowserProtocolFactory : public IClassFactory {
 public:
  static const GUID CLSID_BROWSER_PROTOCOL;

  explicit BrowserProtocolFactory(const Browser::ProtocolHandler& handler);
  ~BrowserProtocolFactory();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IClassFactory
  HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnkOuter,
                                           REFIID riid,
                                           void **ppvObject);
  HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);

 private:
  ULONG ref_;
  Browser::ProtocolHandler handler_;

  DISALLOW_COPY_AND_ASSIGN(BrowserProtocolFactory);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_BROWSER_PROTOCOL_FACTORY_H_
