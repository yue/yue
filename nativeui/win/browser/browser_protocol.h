// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_BROWSER_PROTOCOL_H_
#define NATIVEUI_WIN_BROWSER_BROWSER_PROTOCOL_H_

#include <ole2.h>
#include <urlmon.h>
#include <wrl.h>

#include "nativeui/browser.h"
#include "nativeui/protocol_job.h"

namespace nu {

class BrowserProtocol : public IInternetProtocol,
                        public IInternetProtocolInfo {
 public:
  explicit BrowserProtocol(const Browser::ProtocolHandler& handler);
  virtual ~BrowserProtocol();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IInternetProtocol
  IFACEMETHODIMP Start(LPCWSTR szUrl,
                       IInternetProtocolSink *pIProtSink,
                       IInternetBindInfo *pIBindInfo,
                       DWORD grfSTI,
                       HANDLE_PTR dwReserved);
  IFACEMETHODIMP Continue(PROTOCOLDATA *pStateInfo);
  IFACEMETHODIMP Abort(HRESULT hrReason, DWORD dwOptions);
  IFACEMETHODIMP Terminate(DWORD dwOptions);
  IFACEMETHODIMP Suspend();
  IFACEMETHODIMP Resume();
  IFACEMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
  IFACEMETHODIMP Seek(LARGE_INTEGER dlibMove,
                      DWORD dwOrigin,
                      ULARGE_INTEGER *plibNewPosition);
  IFACEMETHODIMP LockRequest(DWORD dwOptions);
  IFACEMETHODIMP UnlockRequest();

  // IInternetProtocolInfo
  IFACEMETHODIMP ParseUrl(LPCWSTR pwzUrl,
                          PARSEACTION ParseAction,
                          DWORD dwParseFlags,
                          LPWSTR pwzResult,
                          DWORD cchResult,
                          DWORD *pcchResult,
                          DWORD dwReserved);
  IFACEMETHODIMP CombineUrl(LPCWSTR pwzBaseUrl,
                            LPCWSTR pwzRelativeUrl,
                            DWORD dwCombineFlags,
                            LPWSTR pwzResult,
                            DWORD cchResult,
                            DWORD *pcchResult,
                            DWORD dwReserved);
  IFACEMETHODIMP CompareUrl(LPCWSTR pwzUrl1,
                            LPCWSTR pwzUrl2,
                            DWORD dwCompareFlags);
  IFACEMETHODIMP QueryInfo(LPCWSTR pwzUrl,
                           QUERYOPTION QueryOption,
                           DWORD dwQueryFlags,
                           LPVOID pBuffer,
                           DWORD cbBuffer,
                           DWORD *pcbBuf,
                           DWORD dwReserved);

 private:
  ULONG ref_;

  // Managed by BrowserProtocolFactory.
  const Browser::ProtocolHandler& handler_;

  Microsoft::WRL::ComPtr<IInternetProtocolSink> sink_;
  scoped_refptr<ProtocolJob> protocol_job_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_BROWSER_PROTOCOL_H_
