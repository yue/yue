// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_ole_site.h"

#include <shlwapi.h>

#include "nativeui/win/browser/browser_impl_ie.h"

namespace nu {

BrowserOleSite::BrowserOleSite(BrowserImplIE* browser,
                               BrowserExternalSink* external_sink)
    : ref_(0), browser_(browser), external_sink_(external_sink) {
}

BrowserOleSite::~BrowserOleSite() {
}

STDMETHODIMP BrowserOleSite::QueryInterface(REFIID riid, void **ppvObject) {
  const QITAB QITable[] = {
    QITABENT(BrowserOleSite, IOleClientSite),
    QITABENT(BrowserOleSite, IOleInPlaceSite),
    QITABENT(BrowserOleSite, IOleCommandTarget),
    QITABENT(BrowserOleSite, IDocHostUIHandler),
    { 0 },
  };
  return QISearch(this, QITable, riid, ppvObject);
}

STDMETHODIMP_(ULONG) BrowserOleSite::AddRef() {
  return InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) BrowserOleSite::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0) {
    delete this;
  }
  return cref;
}

STDMETHODIMP BrowserOleSite::SaveObject() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::GetMoniker(
    DWORD dwAssign,
    DWORD dwWhichMoniker,
    __RPC__deref_out_opt IMoniker **ppmk) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::GetContainer(
    __RPC__deref_out_opt IOleContainer **ppContainer) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::ShowObject() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::OnShowWindow(BOOL fShow) {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::RequestNewObjectLayout() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::GetWindow(__RPC__deref_out_opt HWND *phwnd) {
  *phwnd = browser_->hwnd();
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::ContextSensitiveHelp(BOOL fEnterMode) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::CanInPlaceActivate() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::OnInPlaceActivate() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::OnUIActivate() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::GetWindowContext(
    __RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
    __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
    __RPC__out LPRECT lprcPosRect,
    __RPC__out LPRECT lprcClipRect,
    __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo) {
  if (ppFrame != nullptr)
    *ppFrame = nullptr;
  if (ppDoc != nullptr)
    *ppDoc = nullptr;
  if (lprcPosRect)
    GetClientRect(browser_->hwnd(), lprcPosRect);
  if (lprcClipRect)
    GetClientRect(browser_->hwnd(), lprcClipRect);
  if (lpFrameInfo) {
    lpFrameInfo->fMDIApp = false;
    lpFrameInfo->hwndFrame = browser_->hwnd();
    lpFrameInfo->haccel = nullptr;
    lpFrameInfo->cAccelEntries = 0;
  }
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::Scroll(SIZE scrollExtant) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::OnUIDeactivate(BOOL fUndoable) {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::OnInPlaceDeactivate() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::DiscardUndoState() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::DeactivateAndUndo() {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::OnPosRectChange(__RPC__in LPCRECT lprcPosRect) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::QueryStatus(const GUID *pguidCmdGroup,
                                           ULONG cCmds,
                                           OLECMD prgCmds[],
                                           OLECMDTEXT *pCmdText) {
  return pguidCmdGroup ? OLECMDERR_E_UNKNOWNGROUP : OLECMDERR_E_NOTSUPPORTED;
}

IFACEMETHODIMP BrowserOleSite::Exec(const GUID *pguidCmdGroup,
                                    DWORD nCmdID,
                                    DWORD nCmdexecopt,
                                    VARIANT *pvaIn,
                                    VARIANT *pvaOut) {
  if (!pguidCmdGroup)
    return OLECMDERR_E_NOTSUPPORTED;
  if (!::IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
    return OLECMDERR_E_UNKNOWNGROUP;
  // Disable builtin dialogs.
  switch (nCmdID) {
    case OLECMDID_SHOWSCRIPTERROR:
    case OLECMDID_SHOWFIND:
    case OLECMDID_SHOWPRINT:
      (*pvaOut).vt = VT_BOOL;
      (*pvaOut).boolVal = VARIANT_TRUE;
      return S_OK;
    default:
      return OLECMDERR_E_NOTSUPPORTED;
  }
}

IFACEMETHODIMP BrowserOleSite::ShowContextMenu(
    _In_ DWORD dwID,
    _In_ POINT *ppt,
    _In_ IUnknown *pcmdtReserved,
    _In_ IDispatch *pdispReserved) {
  if (browser_->options().context_menu)
    return S_FALSE;
  else
    return S_OK;
}

IFACEMETHODIMP BrowserOleSite::GetHostInfo(_Inout_ DOCHOSTUIINFO *pInfo) {
  pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
  pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER |
                   DOCHOSTUIFLAG_DPI_AWARE |
                   DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8 |
                   DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION |
                   DOCHOSTUIFLAG_IME_ENABLE_RECONVERSION |
                   DOCHOSTUIFLAG_THEME |
                   DOCHOSTUIFLAG_USE_WINDOWLESS_SELECTCONTROL;
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::ShowUI(
    DWORD dwID,
    _In_ IOleInPlaceActiveObject *pActiveObject,
    _In_ IOleCommandTarget *pCommandTarget,
    _In_ IOleInPlaceFrame *pFrame,
    _In_ IOleInPlaceUIWindow *pDoc) {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::HideUI() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::UpdateUI() {
  return S_OK;
}

IFACEMETHODIMP BrowserOleSite::EnableModeless(BOOL fEnable) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::OnDocWindowActivate(BOOL fActivate) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::OnFrameWindowActivate(BOOL fActivate) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::ResizeBorder(
    _In_ LPCRECT prcBorder,
    _In_ IOleInPlaceUIWindow *pUIWindow,
    _In_ BOOL fRameWindow) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::TranslateAccelerator(
    LPMSG lpMsg,
    const GUID *pguidCmdGroup,
    DWORD nCmdID) {
  if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_F5)
    return S_OK;
  return S_FALSE;
}

IFACEMETHODIMP BrowserOleSite::GetOptionKeyPath(_Out_ LPOLESTR *pchKey,
                                                DWORD dw) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::GetDropTarget(
    _In_ IDropTarget *pDropTarget,
    _Outptr_ IDropTarget **ppDropTarget) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::GetExternal(
    _Outptr_result_maybenull_ IDispatch **ppDispatch) {
  return external_sink_.CopyTo(ppDispatch);
}

IFACEMETHODIMP BrowserOleSite::TranslateUrl(
    DWORD dwTranslate,
    _In_ LPWSTR pchURLIn,
    _Outptr_ LPWSTR *ppchURLOut) {
  return E_NOTIMPL;
}

IFACEMETHODIMP BrowserOleSite::FilterDataObject(
    _In_ IDataObject *pDO,
    _Outptr_result_maybenull_ IDataObject **ppDORet) {
  return E_NOTIMPL;
}

}  // namespace nu
