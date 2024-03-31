// Copyright 2024 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/win/webview2/browser_protocol_stream.h"

#include <shlwapi.h>

#include <utility>

#include "nativeui/protocol_job.h"

namespace nu {

BrowserProtocolStream::BrowserProtocolStream(scoped_refptr<ProtocolJob> job)
    : protocol_job_(std::move(job)) {}

BrowserProtocolStream::~BrowserProtocolStream() = default;

STDMETHODIMP BrowserProtocolStream::QueryInterface(REFIID riid,
                                                   void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserProtocolStream, ISequentialStream),
    QITABENT(BrowserProtocolStream, IStream),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserProtocolStream::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserProtocolStream::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

IFACEMETHODIMP BrowserProtocolStream::Read(void* pv, ULONG cb, ULONG* pcbRead) {
  *pcbRead = protocol_job_->Read(pv, cb);
  return S_OK;
}

IFACEMETHODIMP BrowserProtocolStream::Write(
    void const* pv, ULONG cb, ULONG* pcbWritten) {
  return STG_E_CANTSAVE;
}

IFACEMETHODIMP BrowserProtocolStream::SetSize(ULARGE_INTEGER) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::CopyTo(
    IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::Commit(DWORD) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::Revert() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::LockRegion(
    ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::UnlockRegion(
    ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::Clone(IStream**) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::Seek(LARGE_INTEGER liDistanceToMove,
                                           DWORD dwOrigin,
                                           ULARGE_INTEGER* lpNewFilePointer) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserProtocolStream::Stat(
    STATSTG* pStatstg, DWORD grfStatFlag) {
  return E_NOTIMPL;
}

}  // namespace nu
