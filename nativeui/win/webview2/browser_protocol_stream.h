// Copyright 2024 Cheng Zhao. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_WIN_WEBVIEW2_BROWSER_PROTOCOL_STREAM_H_
#define NATIVEUI_WIN_WEBVIEW2_BROWSER_PROTOCOL_STREAM_H_

#include <ole2.h>

#include "base/memory/ref_counted.h"

namespace nu {

class ProtocolJob;

class BrowserProtocolStream : public IStream {
 public:
  explicit BrowserProtocolStream(scoped_refptr<ProtocolJob> job);
  virtual ~BrowserProtocolStream();

  BrowserProtocolStream& operator=(const BrowserProtocolStream&) = delete;
  BrowserProtocolStream(const BrowserProtocolStream&) = delete;

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // ISequentialStream
  IFACEMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override;
  IFACEMETHODIMP Write(void const* pv, ULONG cb, ULONG* pcbWritten) override;

  // IStream methods
  IFACEMETHODIMP SetSize(ULARGE_INTEGER) override;
  IFACEMETHODIMP CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*,
                        ULARGE_INTEGER*) override;
  IFACEMETHODIMP Commit(DWORD) override;
  IFACEMETHODIMP Revert() override;
  IFACEMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
  IFACEMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
  IFACEMETHODIMP Clone(IStream**) override;
  IFACEMETHODIMP Seek(LARGE_INTEGER liDistanceToMove,
                      DWORD dwOrigin,
                      ULARGE_INTEGER* lpNewFilePointer) override;
  IFACEMETHODIMP Stat(STATSTG* pStatstg, DWORD grfStatFlag) override;

 private:
  scoped_refptr<ProtocolJob> protocol_job_;
  ULONG ref_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WEBVIEW2_BROWSER_PROTOCOL_STREAM_H_
