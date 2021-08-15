// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_event_sink.h"

#include <exdispid.h>
#include <shlwapi.h>

#include <string>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/browser/browser_impl_ie.h"

namespace nu {

namespace {

// IE might return empty URL for "about:blank";
std::string URLToString(BSTR str) {
  if (::SysStringLen(str) == 0)
    return "about:blank";
  return base::WideToUTF8(str);
}

}  // namespace

BrowserEventSink::BrowserEventSink(BrowserImplIE* browser)
    : ref_(0), browser_(browser) {
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
  auto* delegate = static_cast<Browser*>(browser_->delegate());
  HRESULT hr = S_OK;
  switch (dispIdMember) {
    case DISPID_BEFORENAVIGATE2:
      if (IsMainFrame(pDispParams) && !is_load_html_) {
        delegate->on_start_navigation.Emit(
            delegate,
            URLToString(pDispParams->rgvarg[5].pvarVal->bstrVal));
      }
      break;
    case DISPID_NAVIGATECOMPLETE2:
      // We don't have a way to know when the IE control creates its HWND, our
      // only solution is to keep requesting when navigation state changes.
      browser_->ReceiveBrowserHWND();
      if (IsMainFrame(pDispParams)) {
        // https://msdn.microsoft.com/en-us/library/aa768285(v=vs.85).aspx
        // The viewer for the document has been created.
        browser_->OnDocumentReady();
        // IE does not seem to have the concept of "commit navigation", this is
        // probably the best place to simulate it.
        if (!is_load_html_) {
          delegate->on_commit_navigation.Emit(
              delegate,
              URLToString(pDispParams->rgvarg[0].pvarVal->bstrVal));
        }
        // End loading HTML.
        is_load_html_ = false;
      }
      break;
    case DISPID_DOCUMENTCOMPLETE:
      if (IsMainFrame(pDispParams)) {
        delegate->on_finish_navigation.Emit(
            delegate,
            URLToString(pDispParams->rgvarg[0].pvarVal->bstrVal));
      }
      break;
    case DISPID_NAVIGATEERROR:
      if (IsMainFrame(pDispParams)) {
        delegate->on_fail_navigation.Emit(
            delegate,
            URLToString(pDispParams->rgvarg[3].pvarVal->bstrVal),
            pDispParams->rgvarg[1].pvarVal->intVal);
      }
      // Don't continue to the error page.
      *pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;
      break;
    case DISPID_TITLECHANGE:
      if (!is_load_html_) {
        delegate->on_update_title.Emit(
            delegate,
            base::WideToUTF8(pDispParams->rgvarg[0].bstrVal));
      }
      break;
    case DISPID_COMMANDSTATECHANGE:
      if (pDispParams->rgvarg[1].lVal == CSC_NAVIGATEFORWARD)
        browser_->set_can_go_forward(pDispParams->rgvarg[0].boolVal);
      else if (pDispParams->rgvarg[1].lVal == CSC_NAVIGATEBACK)
        browser_->set_can_go_back(pDispParams->rgvarg[0].boolVal);
      delegate->on_update_command.Emit(delegate);
      break;
    case DISPID_DOWNLOADBEGIN:
    case DISPID_DOWNLOADCOMPLETE:
      delegate->on_change_loading.Emit(delegate);
      break;
    default:
      hr = E_NOTIMPL;
      break;
  }
  return hr;
}

bool BrowserEventSink::IsMainFrame(DISPPARAMS* pDispParams) const {
  size_t last = pDispParams->cArgs - 1;
  if (pDispParams->rgvarg[last].vt != VT_DISPATCH)
    return false;
  Microsoft::WRL::ComPtr<IDispatch> main_window;
  return browser_->GetBrowser<IDispatch>(&main_window) &&
         main_window.Get() == pDispParams->rgvarg[last].pdispVal;
}

}  // namespace nu
