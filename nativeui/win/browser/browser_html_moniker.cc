// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_html_moniker.h"

#include <shlwapi.h>

#include "base/win/scoped_hglobal.h"

namespace nu {

BrowserHTMLMoniker::BrowserHTMLMoniker()
    : ref_(0) {
}

BrowserHTMLMoniker::~BrowserHTMLMoniker() {
}

void BrowserHTMLMoniker::LoadHTML(const std::wstring& str,
                                  const std::wstring& base_url) {
  base_url_ = base_url;
  HGLOBAL glob = ::GlobalAlloc(GPTR, sizeof(wchar_t) * (str.size() + 1));
  {
    base::win::ScopedHGlobal<wchar_t*> global_lock(glob);
    wcscpy_s(global_lock.get(), str.size() + 1, str.c_str());
  }
  ::CreateStreamOnHGlobal(glob, TRUE, &stream_);
}

STDMETHODIMP BrowserHTMLMoniker::QueryInterface(REFIID riid,
                                                void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserHTMLMoniker, IMoniker),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserHTMLMoniker::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserHTMLMoniker::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

STDMETHODIMP BrowserHTMLMoniker::BindToStorage(
    IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riid, void **ppvObj) {
  // Reset the stream cursor to the beginning of the stream.
  LARGE_INTEGER seek = {{0}};
  stream_->Seek(seek, STREAM_SEEK_SET, nullptr);
  return stream_->QueryInterface(riid, ppvObj);
}

STDMETHODIMP BrowserHTMLMoniker::GetDisplayName(
    IBindCtx *pbc, IMoniker *pmkToLeft, LPOLESTR *ppszDisplayName) {
  if (!ppszDisplayName)
    return E_INVALIDARG;
  LPOLESTR display_name = static_cast<LPOLESTR>(
      ::CoTaskMemAlloc(sizeof(wchar_t) * (base_url_.size() + 1)));
  wcscpy_s(display_name, base_url_.size() + 1, base_url_.c_str());
  *ppszDisplayName = display_name;
  return S_OK;
}

STDMETHODIMP BrowserHTMLMoniker::BindToObject(
    IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riidResult, void **ppvResult) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::Reduce(
    IBindCtx *pbc, DWORD dwReduceHowFar, IMoniker **ppmkToLeft,
    IMoniker **ppmkReduced) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::ComposeWith(
    IMoniker *pmkRight, BOOL fOnlyIfNotGeneric, IMoniker **ppmkComposite) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::Enum(
    BOOL fForward, IEnumMoniker **ppenumMoniker) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::IsEqual(IMoniker *pmkOtherMoniker) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::Hash(DWORD *pdwHash) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::IsRunning(
    IBindCtx *pbc, IMoniker *pmkToLeft, IMoniker *pmkNewlyRunning) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::GetTimeOfLastChange(
    IBindCtx *pbc, IMoniker *pmkToLeft, FILETIME *pFileTime) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::Inverse(IMoniker **ppmk) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::CommonPrefixWith(
    IMoniker *pmkOther, IMoniker **ppmkPrefix) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::RelativePathTo(
    IMoniker *pmkOther, IMoniker **ppmkRelPath) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::ParseDisplayName(
    IBindCtx *pbc, IMoniker *pmkToLeft, LPOLESTR pszDisplayName,
    ULONG *pchEaten, IMoniker **ppmkOut) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::IsSystemMoniker(DWORD *pdwMksys) {
  if (!pdwMksys)
    return E_POINTER;
  *pdwMksys = MKSYS_NONE;
  return S_OK;
}

STDMETHODIMP BrowserHTMLMoniker::Save(IStream *pStm, BOOL fClearDirty) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::IsDirty() {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::Load(IStream *pStm) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::GetSizeMax(ULARGE_INTEGER *pcbSize) {
  return E_NOTIMPL;
}

STDMETHODIMP BrowserHTMLMoniker::GetClassID(CLSID *pClassID) {
  return E_NOTIMPL;
}

}  // namespace nu
