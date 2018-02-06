// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_protocol.h"

#include <shlwapi.h>

#include "base/logging.h"

namespace nu {

BrowserProtocol::BrowserProtocol()
    : ref_(1) {
}

BrowserProtocol::~BrowserProtocol() {
}

STDMETHODIMP BrowserProtocol::QueryInterface(REFIID riid,
                                             void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserProtocol, IInternetProtocol),
    QITABENT(BrowserProtocol, IInternetProtocolInfo),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserProtocol::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserProtocol::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

IFACEMETHODIMP BrowserProtocol::Start(LPCWSTR szUrl,
                                      IInternetProtocolSink *pIProtSink,
                                      IInternetBindInfo *pIBindInfo,
                                      DWORD grfSTI,
                                      HANDLE_PTR dwReserved) {
  if (!szUrl || !pIProtSink)
    return E_POINTER;
  pIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE,
                             L"text/html");
  pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION |
                         BSCF_LASTDATANOTIFICATION |
                         BSCF_DATAFULLYAVAILABLE,
                         5, 5);
  pIProtSink->ReportResult(S_OK, 0, 0);
  return S_OK;
}

IFACEMETHODIMP BrowserProtocol::Continue(PROTOCOLDATA *pStateInfo) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Abort(HRESULT hrReason, DWORD dwOptions) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Terminate(DWORD dwOptions) {
  sink_.Reset();
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Suspend() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Resume() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead) {
  static bool r = false;
  if (r)
    return S_FALSE;
  *pcbRead = 5;
  memcpy(pv, "hello", 5);
  r = true;
  return S_OK;
}

IFACEMETHODIMP BrowserProtocol::Seek(LARGE_INTEGER dlibMove,
                                     DWORD dwOrigin,
                                     ULARGE_INTEGER *plibNewPosition) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::LockRequest(DWORD dwOptions) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::UnlockRequest() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocol::ParseUrl(LPCWSTR pwzUrl,
                                         PARSEACTION ParseAction,
                                         DWORD dwParseFlags,
                                         LPWSTR pwzResult,
                                         DWORD cchResult,
                                         DWORD *pcchResult,
                                         DWORD dwReserved) {
  return INET_E_DEFAULT_ACTION;
}

IFACEMETHODIMP BrowserProtocol::CombineUrl(LPCWSTR pwzBaseUrl,
                                           LPCWSTR pwzRelativeUrl,
                                           DWORD dwCombineFlags,
                                           LPWSTR pwzResult,
                                           DWORD cchResult,
                                           DWORD *pcchResult,
                                           DWORD dwReserved) {
  return INET_E_DEFAULT_ACTION;
}

IFACEMETHODIMP BrowserProtocol::CompareUrl(LPCWSTR pwzUrl1,
                                           LPCWSTR pwzUrl2,
                                           DWORD dwCompareFlags) {
  return INET_E_DEFAULT_ACTION;
}

IFACEMETHODIMP BrowserProtocol::QueryInfo(LPCWSTR pwzUrl,
                                          QUERYOPTION QueryOption,
                                          DWORD dwQueryFlags,
                                          LPVOID pBuffer,
                                          DWORD cbBuffer,
                                          DWORD *pcbBuf,
                                          DWORD dwReserved) {
  return INET_E_DEFAULT_ACTION;
}

}  // namespace nu
