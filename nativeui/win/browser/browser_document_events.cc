// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_document_events.h"

#include <unknwn.h>
#include <exdisp.h>  // NOLINT
#include <mshtmdid.h>
#include <mshtml.h>
#include <shlwapi.h>

#include "base/logging.h"

namespace nu {

BrowserDocumentEvents::BrowserDocumentEvents(BrowserImplIE* browser)
    : ref_(0) {}

BrowserDocumentEvents::~BrowserDocumentEvents() {}

STDMETHODIMP BrowserDocumentEvents::QueryInterface(REFIID riid,
                                                   void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserDocumentEvents, IDispatch),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserDocumentEvents::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserDocumentEvents::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

IFACEMETHODIMP BrowserDocumentEvents::GetTypeInfoCount(
    __RPC__out UINT *pctinfo) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserDocumentEvents::GetTypeInfo(
    UINT iTInfo,
    LCID lcid,
    __RPC__deref_out_opt ITypeInfo **ppTInfo) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserDocumentEvents::GetIDsOfNames(
    __RPC__in REFIID riid,
    __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
    __RPC__in_range(0, 16384) UINT cNames,
    LCID lcid,
    __RPC__out_ecount_full(cNames) DISPID *rgDispId) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserDocumentEvents::Invoke(
    _In_ DISPID dispIdMember,
    _In_ REFIID riid,
    _In_ LCID lcid,
    _In_ WORD wFlags,
    _In_ DISPPARAMS *pDispParams,
    _Out_opt_ VARIANT *pVarResult,
    _Out_opt_ EXCEPINFO *pExcepInfo,
    _Out_opt_ UINT *puArgErr) {
  HRESULT hr = S_OK;
  switch (dispIdMember) {
    case DISPID_HTMLDOCUMENTEVENTS2_ONREADYSTATECHANGE:
      break;
    default:
      hr = E_NOTIMPL;
      break;
  }
  return hr;
}

}  // namespace nu
