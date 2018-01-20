// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2017 tokikuch.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser_event_sink.h"

#include <exdispid.h>
#include <shlwapi.h>

#include "nativeui/browser.h"

namespace nu {

BrowserEventSink::BrowserEventSink(Browser* browser)
    : ref_(1), browser_(browser) {
}

BrowserEventSink::~BrowserEventSink() {
}

STDMETHODIMP BrowserEventSink::QueryInterface(REFIID riid, void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserEventSink, IDispatch),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserEventSink::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserEventSink::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

STDMETHODIMP BrowserEventSink::GetTypeInfoCount(__RPC__out UINT *pctinfo) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserEventSink::GetTypeInfo(
    UINT iTInfo,
    LCID lcid,
    __RPC__deref_out_opt ITypeInfo **ppTInfo) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserEventSink::GetIDsOfNames(
    __RPC__in REFIID riid,
    __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
    __RPC__in_range(0, 16384) UINT cNames,
    LCID lcid,
    __RPC__out_ecount_full(cNames) DISPID *rgDispId) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserEventSink::Invoke(_In_  DISPID dispIdMember,
                                      _In_  REFIID riid,
                                      _In_  LCID lcid,
                                      _In_  WORD wFlags,
                                      _In_  DISPPARAMS *pDispParams,
                                      _Out_opt_  VARIANT *pVarResult,
                                      _Out_opt_  EXCEPINFO *pExcepInfo,
                                      _Out_opt_  UINT *puArgErr) {
  HRESULT hr = S_OK;
  switch (dispIdMember) {
    case DISPID_DOCUMENTCOMPLETE:
      break;
    default:
      hr = E_NOTIMPL;
      break;
  }
  return hr;
}

}  // namespace nu
