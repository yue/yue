// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser_external_sink.h"

#include <exdisp.h>
#include <shlwapi.h>

#include "nativeui/win/browser_win.h"

namespace nu {

BrowserExternalSink::BrowserExternalSink(BrowserImpl* browser)
    : ref_(1), browser_(browser) {
}

BrowserExternalSink::~BrowserExternalSink() {
}

STDMETHODIMP BrowserExternalSink::QueryInterface(REFIID riid,
                                                 void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserExternalSink, IDispatch),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserExternalSink::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserExternalSink::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

IFACEMETHODIMP BrowserExternalSink::GetTypeInfoCount(__RPC__out UINT *pctinfo) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserExternalSink::GetTypeInfo(
    UINT iTInfo,
    LCID lcid,
    __RPC__deref_out_opt ITypeInfo **ppTInfo) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserExternalSink::GetIDsOfNames(
    __RPC__in REFIID riid,
    __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
    __RPC__in_range(0, 16384) UINT cNames,
    LCID lcid,
    __RPC__out_ecount_full(cNames) DISPID *rgDispId) {
  return DISP_E_UNKNOWNNAME;
}

IFACEMETHODIMP BrowserExternalSink::Invoke(
    _In_ DISPID dispIdMember,
    _In_ REFIID riid,
    _In_ LCID lcid,
    _In_ WORD wFlags,
    _In_ DISPPARAMS *pDispParams,
    _Out_opt_ VARIANT *pVarResult,
    _Out_opt_ EXCEPINFO *pExcepInfo,
    _Out_opt_ UINT *puArgErr) {
  return E_INVALIDARG;
}

}  // namespace nu
