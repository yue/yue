// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_BROWSER_EVENT_SINK_H_
#define NATIVEUI_WIN_BROWSER_BROWSER_EVENT_SINK_H_

#include <exdisp.h>
#include <ole2.h>

#include "base/macros.h"

namespace nu {

class BrowserImpl;

class BrowserEventSink : public IDispatch {
 public:
  explicit BrowserEventSink(BrowserImpl* browser);
  ~BrowserEventSink();

  // Mark we are loading html string.
  void set_load_html(bool is) { is_load_html_ = is; }

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IDispatch
  IFACEMETHODIMP GetTypeInfoCount(__RPC__out UINT *pctinfo);
  IFACEMETHODIMP GetTypeInfo(UINT iTInfo,
                             LCID lcid,
                             __RPC__deref_out_opt ITypeInfo **ppTInfo);
  IFACEMETHODIMP GetIDsOfNames(
      __RPC__in REFIID riid,
      __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
      __RPC__in_range(0, 16384) UINT cNames,
      LCID lcid,
      __RPC__out_ecount_full(cNames) DISPID *rgDispId);
  IFACEMETHODIMP Invoke(_In_ DISPID dispIdMember,
                        _In_ REFIID riid,
                        _In_ LCID lcid,
                        _In_ WORD wFlags,
                        _In_ DISPPARAMS *pDispParams,
                        _Out_opt_ VARIANT *pVarResult,
                        _Out_opt_ EXCEPINFO *pExcepInfo,
                        _Out_opt_ UINT *puArgErr);

 private:
  bool IsMainFrame(DISPPARAMS* pDispParams) const;

  ULONG ref_;
  BrowserImpl* browser_;

  bool is_load_html_ = false;

  DISALLOW_COPY_AND_ASSIGN(BrowserEventSink);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_BROWSER_EVENT_SINK_H_
