// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_UTIL_MSG_UTIL_H_
#define NATIVEUI_WIN_UTIL_MSG_UTIL_H_

#include "base/logging.h"
#include "nativeui/gfx/geometry/point.h"
#include "nativeui/gfx/geometry/size.h"

// Based on WTL version 8.0 atlcrack.h

// This differs from the original atlcrack.h by removing usage of CPoint,
// CSize, etc.

///////////////////////////////////////////////////////////////////////////////
// Message map macro for cracked handlers

// Note about message maps with cracked handlers:
// For ATL 3.0, a message map using cracked handlers MUST use BEGIN_MSG_MAP_EX.
// For ATL 7.0 or higher, you can use BEGIN_MSG_MAP for CWin32Window/CDialogImpl
// derived classes,
// but must use BEGIN_MSG_MAP_EX for classes that don't derive from
// CWin32Window/CDialogImpl.

#define CR_BEGIN_MSG_MAP_EX(theClass, parent)                      \
 public:                                                           \
  bool m_bMsgHandled;                                              \
  /* "handled" management for cracked handlers */                  \
  bool IsMsgHandled() const { return m_bMsgHandled; }              \
  void SetMsgHandled(bool bHandled) { m_bMsgHandled = bHandled; }  \
  bool ProcessWindowMessage(HWND hWnd,                             \
                            UINT uMsg,                             \
                            WPARAM wParam,                         \
                            LPARAM lParam,                         \
                            LRESULT* result) override {            \
    if (_ProcessWindowMessage(hWnd, uMsg, wParam, lParam, result)) \
      return true;                                                 \
    return parent::ProcessWindowMessage(                           \
        hWnd, uMsg, wParam, lParam, result);                       \
  }                                                                \
  bool _ProcessWindowMessage(HWND hWnd,                            \
                             UINT uMsg,                            \
                             WPARAM wParam,                        \
                             LPARAM lParam,                        \
                             LRESULT* result) {

// Replacement for atlwin.h's END_MSG_MAP for removing ATL usage.
#define CR_END_MSG_MAP()                                          \
    return false;                                                 \
  }

#define CR_GET_X_LPARAM(lParam) ((int)(short)LOWORD(lParam))
#define CR_GET_Y_LPARAM(lParam) ((int)(short)HIWORD(lParam))

///////////////////////////////////////////////////////////////////////////////
// Standard Windows message macros

// int OnCreate(LPCREATESTRUCT lpCreateStruct)
#define CR_MSG_WM_CREATE(func)                       \
  if (uMsg == WM_CREATE) {                           \
    SetMsgHandled(true);                             \
    *result = (LRESULT)func((LPCREATESTRUCT)lParam); \
    if (IsMsgHandled())                              \
      return true;                                   \
  }

// BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
#define CR_MSG_WM_INITDIALOG(func)                 \
  if (uMsg == WM_INITDIALOG) {                     \
    SetMsgHandled(true);                           \
    *result = (LRESULT)func((HWND)wParam, lParam); \
    if (IsMsgHandled())                            \
      return true;                                 \
  }

// BOOL OnCopyData(CWindow wnd, PCOPYDATASTRUCT pCopyDataStruct)
#define CR_MSG_WM_COPYDATA(func)                                    \
  if (uMsg == WM_COPYDATA) {                                        \
    SetMsgHandled(true);                                            \
    *result = (LRESULT)func((HWND)wParam, (PCOPYDATASTRUCT)lParam); \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// void OnDestroy()
#define CR_MSG_WM_DESTROY(func) \
  if (uMsg == WM_DESTROY) {     \
    SetMsgHandled(true);        \
    func();                     \
    *result = 0;                \
    if (IsMsgHandled())         \
      return true;              \
  }

// void OnMove(CPoint ptPos)
#define CR_MSG_WM_MOVE(func)                                            \
  if (uMsg == WM_MOVE) {                                                \
    SetMsgHandled(true);                                                \
    func(nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnSize(UINT nType, nu::Size size)
#define CR_MSG_WM_SIZE(func)                                           \
  if (uMsg == WM_SIZE) {                                               \
    SetMsgHandled(true);                                               \
    func((UINT)wParam,                                                 \
         nu::Size(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                       \
    if (IsMsgHandled())                                                \
      return true;                                                     \
  }

// void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther)
#define CR_MSG_WM_ACTIVATE(func)                                    \
  if (uMsg == WM_ACTIVATE) {                                        \
    SetMsgHandled(true);                                            \
    func((UINT)LOWORD(wParam), (BOOL)HIWORD(wParam), (HWND)lParam); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// void OnSetFocus(CWindow wndOld)
#define CR_MSG_WM_SETFOCUS(func) \
  if (uMsg == WM_SETFOCUS) {     \
    SetMsgHandled(true);         \
    func((HWND)wParam);          \
    *result = 0;                 \
    if (IsMsgHandled())          \
      return true;               \
  }

// void OnKillFocus(CWindow wndFocus)
#define CR_MSG_WM_KILLFOCUS(func) \
  if (uMsg == WM_KILLFOCUS) {     \
    SetMsgHandled(true);          \
    func((HWND)wParam);           \
    *result = 0;                  \
    if (IsMsgHandled())           \
      return true;                \
  }

// void OnEnable(BOOL bEnable)
#define CR_MSG_WM_ENABLE(func) \
  if (uMsg == WM_ENABLE) {     \
    SetMsgHandled(true);       \
    func((BOOL)wParam);        \
    *result = 0;               \
    if (IsMsgHandled())        \
      return true;             \
  }

// void OnPaint(CDCHandle dc)
#define CR_MSG_WM_PAINT(func) \
  if (uMsg == WM_PAINT) {     \
    SetMsgHandled(true);      \
    func((HDC)wParam);        \
    *result = 0;              \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnClose()
#define CR_MSG_WM_CLOSE(func) \
  if (uMsg == WM_CLOSE) {     \
    SetMsgHandled(true);      \
    func();                   \
    *result = 0;              \
    if (IsMsgHandled())       \
      return true;            \
  }

// BOOL OnQueryEndSession(UINT nSource, UINT uLogOff)
#define CR_MSG_WM_QUERYENDSESSION(func)                  \
  if (uMsg == WM_QUERYENDSESSION) {                      \
    SetMsgHandled(true);                                 \
    *result = (LRESULT)func((UINT)wParam, (UINT)lParam); \
    if (IsMsgHandled())                                  \
      return true;                                       \
  }

// BOOL OnQueryOpen()
#define CR_MSG_WM_QUERYOPEN(func) \
  if (uMsg == WM_QUERYOPEN) {     \
    SetMsgHandled(true);          \
    *result = (LRESULT)func();    \
    if (IsMsgHandled())           \
      return true;                \
  }

// BOOL OnEraseBkgnd(CDCHandle dc)
#define CR_MSG_WM_ERASEBKGND(func)        \
  if (uMsg == WM_ERASEBKGND) {            \
    SetMsgHandled(true);                  \
    *result = (LRESULT)func((HDC)wParam); \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// void OnSysColorChange()
#define CR_MSG_WM_SYSCOLORCHANGE(func) \
  if (uMsg == WM_SYSCOLORCHANGE) {     \
    SetMsgHandled(true);               \
    func();                            \
    *result = 0;                       \
    if (IsMsgHandled())                \
      return true;                     \
  }

// void OnEndSession(BOOL bEnding, UINT uLogOff)
#define CR_MSG_WM_ENDSESSION(func)    \
  if (uMsg == WM_ENDSESSION) {        \
    SetMsgHandled(true);              \
    func((BOOL)wParam, (UINT)lParam); \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnShowWindow(BOOL bShow, UINT nStatus)
#define CR_MSG_WM_SHOWWINDOW(func)   \
  if (uMsg == WM_SHOWWINDOW) {       \
    SetMsgHandled(true);             \
    func((BOOL)wParam, (int)lParam); \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

// HBRUSH OnCtlColorEdit(CDCHandle dc, CEdit edit)
#define CR_MSG_WM_CTLCOLOREDIT(func)                    \
  if (uMsg == WM_CTLCOLOREDIT) {                        \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnCtlColorListBox(CDCHandle dc, CListBox listBox)
#define CR_MSG_WM_CTLCOLORLISTBOX(func)                 \
  if (uMsg == WM_CTLCOLORLISTBOX) {                     \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnCtlColorBtn(CDCHandle dc, CButton button)
#define CR_MSG_WM_CTLCOLORBTN(func)                     \
  if (uMsg == WM_CTLCOLORBTN) {                         \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnCtlColorDlg(CDCHandle dc, CWindow wnd)
#define CR_MSG_WM_CTLCOLORDLG(func)                     \
  if (uMsg == WM_CTLCOLORDLG) {                         \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnCtlColorScrollbar(CDCHandle dc, CScrollbar scrollBar)
#define CR_MSG_WM_CTLCOLORSCROLLBAR(func)               \
  if (uMsg == WM_CTLCOLORSCROLLBAR) {                   \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnCtlColorStatic(CDCHandle dc, CStatic wndStatic)
#define CR_MSG_WM_CTLCOLORSTATIC(func)                  \
  if (uMsg == WM_CTLCOLORSTATIC) {                      \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// void OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
#define CR_MSG_WM_SETTINGCHANGE(func)    \
  if (uMsg == WM_SETTINGCHANGE) {        \
    SetMsgHandled(true);                 \
    func((UINT)wParam, (LPCTSTR)lParam); \
    *result = 0;                         \
    if (IsMsgHandled())                  \
      return true;                       \
  }

// void OnDevModeChange(LPCTSTR lpDeviceName)
#define CR_MSG_WM_DEVMODECHANGE(func) \
  if (uMsg == WM_DEVMODECHANGE) {     \
    SetMsgHandled(true);              \
    func((LPCTSTR)lParam);            \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnActivateApp(BOOL bActive, DWORD dwThreadID)
#define CR_MSG_WM_ACTIVATEAPP(func)    \
  if (uMsg == WM_ACTIVATEAPP) {        \
    SetMsgHandled(true);               \
    func((BOOL)wParam, (DWORD)lParam); \
    *result = 0;                       \
    if (IsMsgHandled())                \
      return true;                     \
  }

// void OnFontChange()
#define CR_MSG_WM_FONTCHANGE(func) \
  if (uMsg == WM_FONTCHANGE) {     \
    SetMsgHandled(true);           \
    func();                        \
    *result = 0;                   \
    if (IsMsgHandled())            \
      return true;                 \
  }

// void OnTimeChange()
#define CR_MSG_WM_TIMECHANGE(func) \
  if (uMsg == WM_TIMECHANGE) {     \
    SetMsgHandled(true);           \
    func();                        \
    *result = 0;                   \
    if (IsMsgHandled())            \
      return true;                 \
  }

// void OnCancelMode()
#define CR_MSG_WM_CANCELMODE(func) \
  if (uMsg == WM_CANCELMODE) {     \
    SetMsgHandled(true);           \
    func();                        \
    *result = 0;                   \
    if (IsMsgHandled())            \
      return true;                 \
  }

// BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
#define CR_MSG_WM_SETCURSOR(func)                                  \
  if (uMsg == WM_SETCURSOR) {                                      \
    SetMsgHandled(true);                                           \
    *result = (LRESULT)func(                                       \
        (HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// int OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message)
#define CR_MSG_WM_MOUSEACTIVATE(func)                              \
  if (uMsg == WM_MOUSEACTIVATE) {                                  \
    SetMsgHandled(true);                                           \
    *result = (LRESULT)func(                                       \
        (HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// void OnChildActivate()
#define CR_MSG_WM_CHILDACTIVATE(func) \
  if (uMsg == WM_CHILDACTIVATE) {     \
    SetMsgHandled(true);              \
    func();                           \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
#define CR_MSG_WM_GETMINMAXINFO(func) \
  if (uMsg == WM_GETMINMAXINFO) {     \
    SetMsgHandled(true);              \
    func((LPMINMAXINFO)lParam);       \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnIconEraseBkgnd(CDCHandle dc)
#define CR_MSG_WM_ICONERASEBKGND(func) \
  if (uMsg == WM_ICONERASEBKGND) {     \
    SetMsgHandled(true);               \
    func((HDC)wParam);                 \
    *result = 0;                       \
    if (IsMsgHandled())                \
      return true;                     \
  }

// void OnSpoolerStatus(UINT nStatus, UINT nJobs)
#define CR_MSG_WM_SPOOLERSTATUS(func)         \
  if (uMsg == WM_SPOOLERSTATUS) {             \
    SetMsgHandled(true);                      \
    func((UINT)wParam, (UINT)LOWORD(lParam)); \
    *result = 0;                              \
    if (IsMsgHandled())                       \
      return true;                            \
  }

// void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
#define CR_MSG_WM_DRAWITEM(func)                  \
  if (uMsg == WM_DRAWITEM) {                      \
    SetMsgHandled(true);                          \
    func((UINT)wParam, (LPDRAWITEMSTRUCT)lParam); \
    *result = TRUE;                               \
    if (IsMsgHandled())                           \
      return true;                                \
  }

// void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
#define CR_MSG_WM_MEASUREITEM(func)                  \
  if (uMsg == WM_MEASUREITEM) {                      \
    SetMsgHandled(true);                             \
    func((UINT)wParam, (LPMEASUREITEMSTRUCT)lParam); \
    *result = TRUE;                                  \
    if (IsMsgHandled())                              \
      return true;                                   \
  }

// void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct)
#define CR_MSG_WM_DELETEITEM(func)                  \
  if (uMsg == WM_DELETEITEM) {                      \
    SetMsgHandled(true);                            \
    func((UINT)wParam, (LPDELETEITEMSTRUCT)lParam); \
    *result = TRUE;                                 \
    if (IsMsgHandled())                             \
      return true;                                  \
  }

// int OnCharToItem(UINT nChar, UINT nIndex, CListBox listBox)
#define CR_MSG_WM_CHARTOITEM(func)                                 \
  if (uMsg == WM_CHARTOITEM) {                                     \
    SetMsgHandled(true);                                           \
    *result = (LRESULT)func(                                       \
        (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam); \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// int OnVKeyToItem(UINT nKey, UINT nIndex, CListBox listBox)
#define CR_MSG_WM_VKEYTOITEM(func)                                 \
  if (uMsg == WM_VKEYTOITEM) {                                     \
    SetMsgHandled(true);                                           \
    *result = (LRESULT)func(                                       \
        (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam); \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// HCURSOR OnQueryDragIcon()
#define CR_MSG_WM_QUERYDRAGICON(func) \
  if (uMsg == WM_QUERYDRAGICON) {     \
    SetMsgHandled(true);              \
    *result = (LRESULT)func();        \
    if (IsMsgHandled())               \
      return true;                    \
  }

// int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct)
#define CR_MSG_WM_COMPAREITEM(func)                                     \
  if (uMsg == WM_COMPAREITEM) {                                         \
    SetMsgHandled(true);                                                \
    *result = (LRESULT)func((UINT)wParam, (LPCOMPAREITEMSTRUCT)lParam); \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnCompacting(UINT nCpuTime)
#define CR_MSG_WM_COMPACTING(func) \
  if (uMsg == WM_COMPACTING) {     \
    SetMsgHandled(true);           \
    func((UINT)wParam);            \
    *result = 0;                   \
    if (IsMsgHandled())            \
      return true;                 \
  }

// BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct)
#define CR_MSG_WM_NCCREATE(func)                     \
  if (uMsg == WM_NCCREATE) {                         \
    SetMsgHandled(true);                             \
    *result = (LRESULT)func((LPCREATESTRUCT)lParam); \
    if (IsMsgHandled())                              \
      return true;                                   \
  }

// void OnNcDestroy()
#define CR_MSG_WM_NCDESTROY(func) \
  if (uMsg == WM_NCDESTROY) {     \
    SetMsgHandled(true);          \
    func();                       \
    *result = 0;                  \
    if (IsMsgHandled())           \
      return true;                \
  }

// LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
#define CR_MSG_WM_NCCALCSIZE(func)        \
  if (uMsg == WM_NCCALCSIZE) {            \
    SetMsgHandled(true);                  \
    *result = func((BOOL)wParam, lParam); \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// UINT OnNcHitTest(nu::Point point)
#define CR_MSG_WM_NCHITTEST(func)                                      \
  if (uMsg == WM_NCHITTEST) {                                          \
    SetMsgHandled(true);                                               \
    *result = (LRESULT)func(                                           \
        nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    if (IsMsgHandled())                                                \
      return true;                                                     \
  }

// void OnNcPaint(CRgn rgn)
#define CR_MSG_WM_NCPAINT(func) \
  if (uMsg == WM_NCPAINT) {     \
    SetMsgHandled(true);        \
    func((HRGN)wParam);         \
    *result = 0;                \
    if (IsMsgHandled())         \
      return true;              \
  }

// BOOL OnNcActivate(BOOL bActive)
#define CR_MSG_WM_NCACTIVATE(func)         \
  if (uMsg == WM_NCACTIVATE) {             \
    SetMsgHandled(true);                   \
    *result = (LRESULT)func((BOOL)wParam); \
    if (IsMsgHandled())                    \
      return true;                         \
  }

// UINT OnGetDlgCode(LPMSG lpMsg)
#define CR_MSG_WM_GETDLGCODE(func)          \
  if (uMsg == WM_GETDLGCODE) {              \
    SetMsgHandled(true);                    \
    *result = (LRESULT)func((LPMSG)lParam); \
    if (IsMsgHandled())                     \
      return true;                          \
  }

// void OnNcMouseMove(UINT nHitTest, nu::Point point)
#define CR_MSG_WM_NCMOUSEMOVE(func)                                     \
  if (uMsg == WM_NCMOUSEMOVE) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcLButtonDown(UINT nHitTest, nu::Point point)
#define CR_MSG_WM_NCLBUTTONDOWN(func)                                   \
  if (uMsg == WM_NCLBUTTONDOWN) {                                       \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcLButtonUp(UINT nHitTest, nu::Point point)
#define CR_MSG_WM_NCLBUTTONUP(func)                                     \
  if (uMsg == WM_NCLBUTTONUP) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcLButtonDblClk(UINT nHitTest, nu::Point point)
#define CR_MSG_WM_NCLBUTTONDBLCLK(func)                                 \
  if (uMsg == WM_NCLBUTTONDBLCLK) {                                     \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcRButtonDown(UINT nHitTest, nu::Point point)
#define CR_MSG_WM_NCRBUTTONDOWN(func)                                   \
  if (uMsg == WM_NCRBUTTONDOWN) {                                       \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcRButtonUp(UINT nHitTest, nu::Point point)
#define CR_MSG_WM_NCRBUTTONUP(func)                                     \
  if (uMsg == WM_NCRBUTTONUP) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcRButtonDblClk(UINT nHitTest, CPoint point)
#define CR_MSG_WM_NCRBUTTONDBLCLK(func)                                 \
  if (uMsg == WM_NCRBUTTONDBLCLK) {                                     \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcMButtonDown(UINT nHitTest, CPoint point)
#define CR_MSG_WM_NCMBUTTONDOWN(func)                                   \
  if (uMsg == WM_NCMBUTTONDOWN) {                                       \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcMButtonUp(UINT nHitTest, CPoint point)
#define CR_MSG_WM_NCMBUTTONUP(func)                                     \
  if (uMsg == WM_NCMBUTTONUP) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNcMButtonDblClk(UINT nHitTest, CPoint point)
#define CR_MSG_WM_NCMBUTTONDBLCLK(func)                                 \
  if (uMsg == WM_NCMBUTTONDBLCLK) {                                     \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_KEYDOWN(func)                \
  if (uMsg == WM_KEYDOWN) {                    \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_KEYUP(func)                  \
  if (uMsg == WM_KEYUP) {                      \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_CHAR(func)                   \
  if (uMsg == WM_CHAR) {                       \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_DEADCHAR(func)               \
  if (uMsg == WM_DEADCHAR) {                   \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_SYSKEYDOWN(func)             \
  if (uMsg == WM_SYSKEYDOWN) {                 \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_SYSKEYUP(func)               \
  if (uMsg == WM_SYSKEYUP) {                   \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_SYSCHAR(func)                \
  if (uMsg == WM_SYSCHAR) {                    \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_SYSDEADCHAR(func)            \
  if (uMsg == WM_SYSDEADCHAR) {                \
    SetMsgHandled(true);                       \
    func((TCHAR)wParam,                        \
         (UINT)lParam & 0xFFFF,                \
         (UINT)((lParam & 0xFFFF0000) >> 16)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSysCommand(UINT nID, LPARAM lParam)
#define CR_MSG_WM_SYSCOMMAND(func)                                      \
  if (uMsg == WM_SYSCOMMAND) {                                          \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnTCard(UINT idAction, DWORD dwActionData)
#define CR_MSG_WM_TCARD(func)          \
  if (uMsg == WM_TCARD) {              \
    SetMsgHandled(true);               \
    func((UINT)wParam, (DWORD)lParam); \
    *result = 0;                       \
    if (IsMsgHandled())                \
      return true;                     \
  }

// void OnTimer(UINT_PTR nIDEvent)
#define CR_MSG_WM_TIMER(func) \
  if (uMsg == WM_TIMER) {     \
    SetMsgHandled(true);      \
    func((UINT_PTR)wParam);   \
    *result = 0;              \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnHScroll(UINT nSBCode, UINT nPos, CScrollbar pScrollbar)
#define CR_MSG_WM_HSCROLL(func)                                     \
  if (uMsg == WM_HSCROLL) {                                         \
    SetMsgHandled(true);                                            \
    func((int)LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// void OnVScroll(UINT nSBCode, UINT nPos, CScrollbar pScrollbar)
#define CR_MSG_WM_VSCROLL(func)                                     \
  if (uMsg == WM_VSCROLL) {                                         \
    SetMsgHandled(true);                                            \
    func((int)LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// void OnInitMenu(CMenu menu)
#define CR_MSG_WM_INITMENU(func) \
  if (uMsg == WM_INITMENU) {     \
    SetMsgHandled(true);         \
    func((HMENU)wParam);         \
    *result = 0;                 \
    if (IsMsgHandled())          \
      return true;               \
  }

// void OnInitMenuPopup(CMenu menuPopup, UINT nIndex, BOOL bSysMenu)
#define CR_MSG_WM_INITMENUPOPUP(func)                                \
  if (uMsg == WM_INITMENUPOPUP) {                                    \
    SetMsgHandled(true);                                             \
    func((HMENU)wParam, (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)); \
    *result = 0;                                                     \
    if (IsMsgHandled())                                              \
      return true;                                                   \
  }

// void OnMenuSelect(UINT nItemID, UINT nFlags, CMenu menu)
#define CR_MSG_WM_MENUSELECT(func)                                   \
  if (uMsg == WM_MENUSELECT) {                                       \
    SetMsgHandled(true);                                             \
    func((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HMENU)lParam); \
    *result = 0;                                                     \
    if (IsMsgHandled())                                              \
      return true;                                                   \
  }

// LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu menu)
#define CR_MSG_WM_MENUCHAR(func)                                          \
  if (uMsg == WM_MENUCHAR) {                                              \
    SetMsgHandled(true);                                                  \
    *result =                                                             \
        func((TCHAR)LOWORD(wParam), (UINT)HIWORD(wParam), (HMENU)lParam); \
    if (IsMsgHandled())                                                   \
      return true;                                                        \
  }

// LRESULT OnNotify(int idCtrl, LPNMHDR pnmh)
#define CR_MSG_WM_NOTIFY(func)                    \
  if (uMsg == WM_NOTIFY) {                        \
    SetMsgHandled(true);                          \
    *result = func((int)wParam, (LPNMHDR)lParam); \
    if (IsMsgHandled())                           \
      return true;                                \
  }

// void OnEnterIdle(UINT nWhy, CWindow wndWho)
#define CR_MSG_WM_ENTERIDLE(func)     \
  if (uMsg == WM_ENTERIDLE) {         \
    SetMsgHandled(true);              \
    func((UINT)wParam, (HWND)lParam); \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnMouseMove(UINT nFlags, CPoint point)
#define CR_MSG_WM_MOUSEMOVE(func)                                       \
  if (uMsg == WM_MOUSEMOVE) {                                           \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// BOOL OnMouseWheel(bool vertical, UINT nFlags, short zDelta, CPoint pt)
#define CR_MSG_WM_MOUSEWHEEL(func)                                     \
  if (uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEHWHEEL) {               \
    SetMsgHandled(true);                                               \
    *result = (LRESULT)func(                                           \
        uMsg == WM_MOUSEWHEEL,                                         \
        (UINT)LOWORD(wParam),                                          \
        (short)HIWORD(wParam),                                         \
        nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    if (IsMsgHandled())                                                \
      return true;                                                     \
  }

// void OnLButtonDown(UINT nFlags, CPoint point)
#define CR_MSG_WM_LBUTTONDOWN(func)                                     \
  if (uMsg == WM_LBUTTONDOWN) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnLButtonUp(UINT nFlags, CPoint point)
#define CR_MSG_WM_LBUTTONUP(func)                                       \
  if (uMsg == WM_LBUTTONUP) {                                           \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnLButtonDblClk(UINT nFlags, CPoint point)
#define CR_MSG_WM_LBUTTONDBLCLK(func)                                   \
  if (uMsg == WM_LBUTTONDBLCLK) {                                       \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnRButtonDown(UINT nFlags, CPoint point)
#define CR_MSG_WM_RBUTTONDOWN(func)                                     \
  if (uMsg == WM_RBUTTONDOWN) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnRButtonUp(UINT nFlags, CPoint point)
#define CR_MSG_WM_RBUTTONUP(func)                                       \
  if (uMsg == WM_RBUTTONUP) {                                           \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnRButtonDblClk(UINT nFlags, CPoint point)
#define CR_MSG_WM_RBUTTONDBLCLK(func)                                   \
  if (uMsg == WM_RBUTTONDBLCLK) {                                       \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnMButtonDown(UINT nFlags, CPoint point)
#define CR_MSG_WM_MBUTTONDOWN(func)                                     \
  if (uMsg == WM_MBUTTONDOWN) {                                         \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnMButtonUp(UINT nFlags, CPoint point)
#define CR_MSG_WM_MBUTTONUP(func)                                       \
  if (uMsg == WM_MBUTTONUP) {                                           \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnMButtonDblClk(UINT nFlags, CPoint point)
#define CR_MSG_WM_MBUTTONDBLCLK(func)                                   \
  if (uMsg == WM_MBUTTONDBLCLK) {                                       \
    SetMsgHandled(true);                                                \
    func((UINT)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnParentNotify(UINT message, UINT nChildID, LPARAM lParam)
#define CR_MSG_WM_PARENTNOTIFY(func)                          \
  if (uMsg == WM_PARENTNOTIFY) {                              \
    SetMsgHandled(true);                                      \
    func((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), lParam); \
    *result = 0;                                              \
    if (IsMsgHandled())                                       \
      return true;                                            \
  }

// void OnMDIActivate(CWindow wndActivate, CWindow wndDeactivate)
#define CR_MSG_WM_MDIACTIVATE(func)   \
  if (uMsg == WM_MDIACTIVATE) {       \
    SetMsgHandled(true);              \
    func((HWND)wParam, (HWND)lParam); \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnRenderFormat(UINT nFormat)
#define CR_MSG_WM_RENDERFORMAT(func) \
  if (uMsg == WM_RENDERFORMAT) {     \
    SetMsgHandled(true);             \
    func((UINT)wParam);              \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

// void OnRenderAllFormats()
#define CR_MSG_WM_RENDERALLFORMATS(func) \
  if (uMsg == WM_RENDERALLFORMATS) {     \
    SetMsgHandled(true);                 \
    func();                              \
    *result = 0;                         \
    if (IsMsgHandled())                  \
      return true;                       \
  }

// void OnDestroyClipboard()
#define CR_MSG_WM_DESTROYCLIPBOARD(func) \
  if (uMsg == WM_DESTROYCLIPBOARD) {     \
    SetMsgHandled(true);                 \
    func();                              \
    *result = 0;                         \
    if (IsMsgHandled())                  \
      return true;                       \
  }

// void OnDrawClipboard()
#define CR_MSG_WM_DRAWCLIPBOARD(func) \
  if (uMsg == WM_DRAWCLIPBOARD) {     \
    SetMsgHandled(true);              \
    func();                           \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnPaintClipboard(CWindow wndViewer, const LPPAINTSTRUCT lpPaintStruct)
#define CR_MSG_WM_PAINTCLIPBOARD(func)                                      \
  if (uMsg == WM_PAINTCLIPBOARD) {                                          \
    SetMsgHandled(true);                                                    \
    func((HWND)wParam, (const LPPAINTSTRUCT)::GlobalLock((HGLOBAL)lParam)); \
    ::GlobalUnlock((HGLOBAL)lParam);                                        \
    *result = 0;                                                            \
    if (IsMsgHandled())                                                     \
      return true;                                                          \
  }

// void OnVScrollClipboard(CWindow wndViewer, UINT nSBCode, UINT nPos)
#define CR_MSG_WM_VSCROLLCLIPBOARD(func)                            \
  if (uMsg == WM_VSCROLLCLIPBOARD) {                                \
    SetMsgHandled(true);                                            \
    func((HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// void OnContextMenu(CWindow wnd, CPoint point)
#define CR_MSG_WM_CONTEXTMENU(func)                                     \
  if (uMsg == WM_CONTEXTMENU) {                                         \
    SetMsgHandled(true);                                                \
    func((HWND)wParam,                                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnSizeClipboard(CWindow wndViewer, const LPRECT lpRect)
#define CR_MSG_WM_SIZECLIPBOARD(func)                                \
  if (uMsg == WM_SIZECLIPBOARD) {                                    \
    SetMsgHandled(true);                                             \
    func((HWND)wParam, (const LPRECT)::GlobalLock((HGLOBAL)lParam)); \
    ::GlobalUnlock((HGLOBAL)lParam);                                 \
    *result = 0;                                                     \
    if (IsMsgHandled())                                              \
      return true;                                                   \
  }

// void OnAskCbFormatName(UINT nMaxCount, LPTSTR lpszString)
#define CR_MSG_WM_ASKCBFORMATNAME(func)  \
  if (uMsg == WM_ASKCBFORMATNAME) {      \
    SetMsgHandled(true);                 \
    func((DWORD)wParam, (LPTSTR)lParam); \
    *result = 0;                         \
    if (IsMsgHandled())                  \
      return true;                       \
  }

// void OnChangeCbChain(CWindow wndRemove, CWindow wndAfter)
#define CR_MSG_WM_CHANGECBCHAIN(func) \
  if (uMsg == WM_CHANGECBCHAIN) {     \
    SetMsgHandled(true);              \
    func((HWND)wParam, (HWND)lParam); \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnHScrollClipboard(CWindow wndViewer, UINT nSBCode, UINT nPos)
#define CR_MSG_WM_HSCROLLCLIPBOARD(func)                            \
  if (uMsg == WM_HSCROLLCLIPBOARD) {                                \
    SetMsgHandled(true);                                            \
    func((HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// BOOL OnQueryNewPalette()
#define CR_MSG_WM_QUERYNEWPALETTE(func) \
  if (uMsg == WM_QUERYNEWPALETTE) {     \
    SetMsgHandled(true);                \
    *result = (LRESULT)func();          \
    if (IsMsgHandled())                 \
      return true;                      \
  }

// void OnPaletteChanged(CWindow wndFocus)
#define CR_MSG_WM_PALETTECHANGED(func) \
  if (uMsg == WM_PALETTECHANGED) {     \
    SetMsgHandled(true);               \
    func((HWND)wParam);                \
    *result = 0;                       \
    if (IsMsgHandled())                \
      return true;                     \
  }

// void OnPaletteIsChanging(CWindow wndPalChg)
#define CR_MSG_WM_PALETTEISCHANGING(func) \
  if (uMsg == WM_PALETTEISCHANGING) {     \
    SetMsgHandled(true);                  \
    func((HWND)wParam);                   \
    *result = 0;                          \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// void OnDropFiles(HDROP hDropInfo)
#define CR_MSG_WM_DROPFILES(func) \
  if (uMsg == WM_DROPFILES) {     \
    SetMsgHandled(true);          \
    func((HDROP)wParam);          \
    *result = 0;                  \
    if (IsMsgHandled())           \
      return true;                \
  }

// void OnWindowPosChanging(LPWINDOWPOS lpWndPos)
#define CR_MSG_WM_WINDOWPOSCHANGING(func) \
  if (uMsg == WM_WINDOWPOSCHANGING) {     \
    SetMsgHandled(true);                  \
    func((LPWINDOWPOS)lParam);            \
    *result = 0;                          \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
#define CR_MSG_WM_WINDOWPOSCHANGED(func) \
  if (uMsg == WM_WINDOWPOSCHANGED) {     \
    SetMsgHandled(true);                 \
    func((LPWINDOWPOS)lParam);           \
    *result = 0;                         \
    if (IsMsgHandled())                  \
      return true;                       \
  }

// void OnExitMenuLoop(BOOL fIsTrackPopupMenu)
#define CR_MSG_WM_EXITMENULOOP(func) \
  if (uMsg == WM_EXITMENULOOP) {     \
    SetMsgHandled(true);             \
    func((BOOL)wParam);              \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

// void OnEnterMenuLoop(BOOL fIsTrackPopupMenu)
#define CR_MSG_WM_ENTERMENULOOP(func) \
  if (uMsg == WM_ENTERMENULOOP) {     \
    SetMsgHandled(true);              \
    func((BOOL)wParam);               \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
#define CR_MSG_WM_STYLECHANGED(func)           \
  if (uMsg == WM_STYLECHANGED) {               \
    SetMsgHandled(true);                       \
    func((UINT)wParam, (LPSTYLESTRUCT)lParam); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
#define CR_MSG_WM_STYLECHANGING(func)          \
  if (uMsg == WM_STYLECHANGING) {              \
    SetMsgHandled(true);                       \
    func((UINT)wParam, (LPSTYLESTRUCT)lParam); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSizing(UINT fwSide, LPRECT pRect)
#define CR_MSG_WM_SIZING(func)          \
  if (uMsg == WM_SIZING) {              \
    SetMsgHandled(true);                \
    func((UINT)wParam, (LPRECT)lParam); \
    *result = TRUE;                     \
    if (IsMsgHandled())                 \
      return true;                      \
  }

// void OnMoving(UINT fwSide, LPRECT pRect)
#define CR_MSG_WM_MOVING(func)          \
  if (uMsg == WM_MOVING) {              \
    SetMsgHandled(true);                \
    func((UINT)wParam, (LPRECT)lParam); \
    *result = TRUE;                     \
    if (IsMsgHandled())                 \
      return true;                      \
  }

// void OnCaptureChanged(CWindow wnd)
#define CR_MSG_WM_CAPTURECHANGED(func) \
  if (uMsg == WM_CAPTURECHANGED) {     \
    SetMsgHandled(true);               \
    func((HWND)lParam);                \
    *result = 0;                       \
    if (IsMsgHandled())                \
      return true;                     \
  }

// BOOL OnDeviceChange(UINT nEventType, DWORD dwData)
#define CR_MSG_WM_DEVICECHANGE(func)                      \
  if (uMsg == WM_DEVICECHANGE) {                          \
    SetMsgHandled(true);                                  \
    *result = (LRESULT)func((UINT)wParam, (DWORD)lParam); \
    if (IsMsgHandled())                                   \
      return true;                                        \
  }

// void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
#define CR_MSG_WM_COMMAND(func)                                    \
  if (uMsg == WM_COMMAND) {                                        \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// void OnDisplayChange(UINT uBitsPerPixel, nu::Size sizeScreen)
#define CR_MSG_WM_DISPLAYCHANGE(func)                                  \
  if (uMsg == WM_DISPLAYCHANGE) {                                      \
    SetMsgHandled(true);                                               \
    func((UINT)wParam,                                                 \
         nu::Size(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                       \
    if (IsMsgHandled())                                                \
      return true;                                                     \
  }

// void OnEnterSizeMove()
#define CR_MSG_WM_ENTERSIZEMOVE(func) \
  if (uMsg == WM_ENTERSIZEMOVE) {     \
    SetMsgHandled(true);              \
    func();                           \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnExitSizeMove()
#define CR_MSG_WM_EXITSIZEMOVE(func) \
  if (uMsg == WM_EXITSIZEMOVE) {     \
    SetMsgHandled(true);             \
    func();                          \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

// HFONT OnGetFont()
#define CR_MSG_WM_GETFONT(func) \
  if (uMsg == WM_GETFONT) {     \
    SetMsgHandled(true);        \
    *result = (LRESULT)func();  \
    if (IsMsgHandled())         \
      return true;              \
  }

// LRESULT OnGetHotKey()
#define CR_MSG_WM_GETHOTKEY(func) \
  if (uMsg == WM_GETHOTKEY) {     \
    SetMsgHandled(true);          \
    *result = func();             \
    if (IsMsgHandled())           \
      return true;                \
  }

// HICON OnGetIcon()
#define CR_MSG_WM_GETICON(func)            \
  if (uMsg == WM_GETICON) {                \
    SetMsgHandled(true);                   \
    *result = (LRESULT)func((UINT)wParam); \
    if (IsMsgHandled())                    \
      return true;                         \
  }

// int OnGetText(int cchTextMax, LPTSTR lpszText)
#define CR_MSG_WM_GETTEXT(func)                           \
  if (uMsg == WM_GETTEXT) {                               \
    SetMsgHandled(true);                                  \
    *result = (LRESULT)func((int)wParam, (LPTSTR)lParam); \
    if (IsMsgHandled())                                   \
      return true;                                        \
  }

// int OnGetTextLength()
#define CR_MSG_WM_GETTEXTLENGTH(func) \
  if (uMsg == WM_GETTEXTLENGTH) {     \
    SetMsgHandled(true);              \
    *result = (LRESULT)func();        \
    if (IsMsgHandled())               \
      return true;                    \
  }

// void OnHelp(LPHELPINFO lpHelpInfo)
#define CR_MSG_WM_HELP(func)  \
  if (uMsg == WM_HELP) {      \
    SetMsgHandled(true);      \
    func((LPHELPINFO)lParam); \
    *result = TRUE;           \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey)
#define CR_MSG_WM_HOTKEY(func)                                     \
  if (uMsg == WM_HOTKEY) {                                         \
    SetMsgHandled(true);                                           \
    func((int)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// void OnInputLangChange(DWORD dwCharSet, HKL hKbdLayout)
#define CR_MSG_WM_INPUTLANGCHANGE(func) \
  if (uMsg == WM_INPUTLANGCHANGE) {     \
    SetMsgHandled(true);                \
    func((DWORD)wParam, (HKL)lParam);   \
    *result = TRUE;                     \
    if (IsMsgHandled())                 \
      return true;                      \
  }

// void OnInputLangChangeRequest(BOOL bSysCharSet, HKL hKbdLayout)
#define CR_MSG_WM_INPUTLANGCHANGEREQUEST(func) \
  if (uMsg == WM_INPUTLANGCHANGEREQUEST) {     \
    SetMsgHandled(true);                       \
    func((BOOL)wParam, (HKL)lParam);           \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnNextDlgCtl(BOOL bHandle, WPARAM wCtlFocus)
#define CR_MSG_WM_NEXTDLGCTL(func)      \
  if (uMsg == WM_NEXTDLGCTL) {          \
    SetMsgHandled(true);                \
    func((BOOL)LOWORD(lParam), wParam); \
    *result = 0;                        \
    if (IsMsgHandled())                 \
      return true;                      \
  }

// void OnNextMenu(int nVirtKey, LPMDINEXTMENU lpMdiNextMenu)
#define CR_MSG_WM_NEXTMENU(func)              \
  if (uMsg == WM_NEXTMENU) {                  \
    SetMsgHandled(true);                      \
    func((int)wParam, (LPMDINEXTMENU)lParam); \
    *result = 0;                              \
    if (IsMsgHandled())                       \
      return true;                            \
  }

// int OnNotifyFormat(CWindow wndFrom, int nCommand)
#define CR_MSG_WM_NOTIFYFORMAT(func)                    \
  if (uMsg == WM_NOTIFYFORMAT) {                        \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HWND)wParam, (int)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// BOOL OnPowerBroadcast(DWORD dwPowerEvent, DWORD dwData)
#define CR_MSG_WM_POWERBROADCAST(func)                     \
  if (uMsg == WM_POWERBROADCAST) {                         \
    SetMsgHandled(true);                                   \
    *result = (LRESULT)func((DWORD)wParam, (DWORD)lParam); \
    if (IsMsgHandled())                                    \
      return true;                                         \
  }

// void OnPrint(CDCHandle dc, UINT uFlags)
#define CR_MSG_WM_PRINT(func)        \
  if (uMsg == WM_PRINT) {            \
    SetMsgHandled(true);             \
    func((HDC)wParam, (UINT)lParam); \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

// void OnPrintClient(CDCHandle dc, UINT uFlags)
#define CR_MSG_WM_PRINTCLIENT(func)  \
  if (uMsg == WM_PRINTCLIENT) {      \
    SetMsgHandled(true);             \
    func((HDC)wParam, (UINT)lParam); \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

// void OnRasDialEvent(RASCONNSTATE rasconnstate, DWORD dwError)
#define CR_MSG_WM_RASDIALEVENT(func)           \
  if (uMsg == WM_RASDIALEVENT) {               \
    SetMsgHandled(true);                       \
    func((RASCONNSTATE)wParam, (DWORD)lParam); \
    *result = TRUE;                            \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnSetFont(CFont font, BOOL bRedraw)
#define CR_MSG_WM_SETFONT(func)                \
  if (uMsg == WM_SETFONT) {                    \
    SetMsgHandled(true);                       \
    func((HFONT)wParam, (BOOL)LOWORD(lParam)); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// int OnSetHotKey(int nVirtKey, UINT uFlags)
#define CR_MSG_WM_SETHOTKEY(func)                          \
  if (uMsg == WM_SETHOTKEY) {                              \
    SetMsgHandled(true);                                   \
    *result = (LRESULT)func((int)LOBYTE(LOWORD(wParam)),   \
                            (UINT)HIBYTE(LOWORD(wParam))); \
    if (IsMsgHandled())                                    \
      return true;                                         \
  }

// HICON OnSetIcon(UINT uType, HICON hIcon)
#define CR_MSG_WM_SETICON(func)                           \
  if (uMsg == WM_SETICON) {                               \
    SetMsgHandled(true);                                  \
    *result = (LRESULT)func((UINT)wParam, (HICON)lParam); \
    if (IsMsgHandled())                                   \
      return true;                                        \
  }

// void OnSetRedraw(BOOL bRedraw)
#define CR_MSG_WM_SETREDRAW(func) \
  if (uMsg == WM_SETREDRAW) {     \
    SetMsgHandled(true);          \
    func((BOOL)wParam);           \
    *result = 0;                  \
    if (IsMsgHandled())           \
      return true;                \
  }

// int OnSetText(LPCTSTR lpstrText)
#define CR_MSG_WM_SETTEXT(func)               \
  if (uMsg == WM_SETTEXT) {                   \
    SetMsgHandled(true);                      \
    *result = (LRESULT)func((LPCTSTR)lParam); \
    if (IsMsgHandled())                       \
      return true;                            \
  }

// void OnUserChanged()
#define CR_MSG_WM_USERCHANGED(func) \
  if (uMsg == WM_USERCHANGED) {     \
    SetMsgHandled(true);            \
    func();                         \
    *result = 0;                    \
    if (IsMsgHandled())             \
      return true;                  \
  }

///////////////////////////////////////////////////////////////////////////////
// New NT4 & NT5 messages

#if (_WIN32_WINNT >= 0x0400)

// void OnMouseHover(WPARAM wParam, CPoint ptPos)
#define CR_MSG_WM_MOUSEHOVER(func)                                      \
  if (uMsg == WM_MOUSEHOVER) {                                          \
    SetMsgHandled(true);                                                \
    func(wParam,                                                        \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnMouseLeave()
#define CR_MSG_WM_MOUSELEAVE(func) \
  if (uMsg == WM_MOUSELEAVE) {     \
    SetMsgHandled(true);           \
    func();                        \
    *result = 0;                   \
    if (IsMsgHandled())            \
      return true;                 \
  }

#endif /* _WIN32_WINNT >= 0x0400 */

#if (WINVER >= 0x0500)

// void OnMenuRButtonUp(WPARAM wParam, CMenu menu)
#define CR_MSG_WM_MENURBUTTONUP(func) \
  if (uMsg == WM_MENURBUTTONUP) {     \
    SetMsgHandled(true);              \
    func(wParam, (HMENU)lParam);      \
    *result = 0;                      \
    if (IsMsgHandled())               \
      return true;                    \
  }

// LRESULT OnMenuDrag(WPARAM wParam, CMenu menu)
#define CR_MSG_WM_MENUDRAG(func)           \
  if (uMsg == WM_MENUDRAG) {               \
    SetMsgHandled(true);                   \
    *result = func(wParam, (HMENU)lParam); \
    if (IsMsgHandled())                    \
      return true;                         \
  }

// LRESULT OnMenuGetObject(PMENUGETOBJECTINFO info)
#define CR_MSG_WM_MENUGETOBJECT(func)           \
  if (uMsg == WM_MENUGETOBJECT) {               \
    SetMsgHandled(true);                        \
    *result = func((PMENUGETOBJECTINFO)lParam); \
    if (IsMsgHandled())                         \
      return true;                              \
  }

// void OnUnInitMenuPopup(UINT nID, CMenu menu)
#define CR_MSG_WM_UNINITMENUPOPUP(func)        \
  if (uMsg == WM_UNINITMENUPOPUP) {            \
    SetMsgHandled(true);                       \
    func((UINT)HIWORD(lParam), (HMENU)wParam); \
    *result = 0;                               \
    if (IsMsgHandled())                        \
      return true;                             \
  }

// void OnMenuCommand(WPARAM nIndex, CMenu menu)
#define CR_MSG_WM_MENUCOMMAND(func) \
  if (uMsg == WM_MENUCOMMAND) {     \
    SetMsgHandled(true);            \
    func(wParam, (HMENU)lParam);    \
    *result = 0;                    \
    if (IsMsgHandled())             \
      return true;                  \
  }

#endif /* WINVER >= 0x0500 */

#if (_WIN32_WINNT >= 0x0500)

// BOOL OnAppCommand(CWindow wndFocus, short cmd, WORD uDevice, int dwKeys)
#define CR_MSG_WM_APPCOMMAND(func)                         \
  if (uMsg == WM_APPCOMMAND) {                             \
    SetMsgHandled(true);                                   \
    *result = (LRESULT)func((HWND)wParam,                  \
                            GET_APPCOMMAND_LPARAM(lParam), \
                            GET_DEVICE_LPARAM(lParam),     \
                            GET_KEYSTATE_LPARAM(lParam));  \
    if (IsMsgHandled())                                    \
      return true;                                         \
  }

// void OnNCXButtonDown(int fwButton, short nHittest, CPoint ptPos)
#define CR_MSG_WM_NCXBUTTONDOWN(func)                                   \
  if (uMsg == WM_NCXBUTTONDOWN) {                                       \
    SetMsgHandled(true);                                                \
    func(GET_XBUTTON_WPARAM(wParam),                                    \
         GET_NCHITTEST_WPARAM(wParam),                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNCXButtonUp(int fwButton, short nHittest, CPoint ptPos)
#define CR_MSG_WM_NCXBUTTONUP(func)                                     \
  if (uMsg == WM_NCXBUTTONUP) {                                         \
    SetMsgHandled(true);                                                \
    func(GET_XBUTTON_WPARAM(wParam),                                    \
         GET_NCHITTEST_WPARAM(wParam),                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnNCXButtonDblClk(int fwButton, short nHittest, CPoint ptPos)
#define CR_MSG_WM_NCXBUTTONDBLCLK(func)                                 \
  if (uMsg == WM_NCXBUTTONDBLCLK) {                                     \
    SetMsgHandled(true);                                                \
    func(GET_XBUTTON_WPARAM(wParam),                                    \
         GET_NCHITTEST_WPARAM(wParam),                                  \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnXButtonDown(int fwButton, int dwKeys, CPoint ptPos)
#define CR_MSG_WM_XBUTTONDOWN(func)                                     \
  if (uMsg == WM_XBUTTONDOWN) {                                         \
    SetMsgHandled(true);                                                \
    func(GET_XBUTTON_WPARAM(wParam),                                    \
         GET_KEYSTATE_WPARAM(wParam),                                   \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnXButtonUp(int fwButton, int dwKeys, CPoint ptPos)
#define CR_MSG_WM_XBUTTONUP(func)                                       \
  if (uMsg == WM_XBUTTONUP) {                                           \
    SetMsgHandled(true);                                                \
    func(GET_XBUTTON_WPARAM(wParam),                                    \
         GET_KEYSTATE_WPARAM(wParam),                                   \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnXButtonDblClk(int fwButton, int dwKeys, CPoint ptPos)
#define CR_MSG_WM_XBUTTONDBLCLK(func)                                   \
  if (uMsg == WM_XBUTTONDBLCLK) {                                       \
    SetMsgHandled(true);                                                \
    func(GET_XBUTTON_WPARAM(wParam),                                    \
         GET_KEYSTATE_WPARAM(wParam),                                   \
         nu::Point(CR_GET_X_LPARAM(lParam), CR_GET_Y_LPARAM(lParam)));  \
    *result = 0;                                                        \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnChangeUIState(WORD nAction, WORD nState)
#define CR_MSG_WM_CHANGEUISTATE(func)     \
  if (uMsg == WM_CHANGEUISTATE) {         \
    SetMsgHandled(true);                  \
    func(LOWORD(wParam), HIWORD(wParam)); \
    *result = 0;                          \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// void OnUpdateUIState(WORD nAction, WORD nState)
#define CR_MSG_WM_UPDATEUISTATE(func)     \
  if (uMsg == WM_UPDATEUISTATE) {         \
    SetMsgHandled(true);                  \
    func(LOWORD(wParam), HIWORD(wParam)); \
    *result = 0;                          \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// LRESULT OnQueryUIState()
#define CR_MSG_WM_QUERYUISTATE(func) \
  if (uMsg == WM_QUERYUISTATE) {     \
    SetMsgHandled(true);             \
    *result = func();                \
    if (IsMsgHandled())              \
      return true;                   \
  }

#endif  // (_WIN32_WINNT >= 0x0500)

#if (_WIN32_WINNT >= 0x0501)

// void OnInput(WPARAM RawInputCode, HRAWINPUT hRawInput)
#define CR_MSG_WM_INPUT(func)                                  \
  if (uMsg == WM_INPUT) {                                      \
    SetMsgHandled(true);                                       \
    func(GET_RAWINPUT_CODE_WPARAM(wParam), (HRAWINPUT)lParam); \
    *result = 0;                                               \
    if (IsMsgHandled())                                        \
      return true;                                             \
  }

// void OnUniChar(TCHAR nChar, UINT nRepCnt, UINT nFlags)
#define CR_MSG_WM_UNICHAR(func)                            \
  if (uMsg == WM_UNICHAR) {                                \
    SetMsgHandled(true);                                   \
    func((TCHAR)wParam,                                    \
         (UINT)lParam & 0xFFFF,                            \
         (UINT)((lParam & 0xFFFF0000) >> 16));             \
    if (IsMsgHandled()) {                                  \
      *result = (wParam == UNICODE_NOCHAR) ? TRUE : FALSE; \
      return true;                                         \
    }                                                      \
  }

// OnThemeChanged()
#define CR_MSG_WM_THEMECHANGED(func) \
  if (uMsg == WM_THEMECHANGED) {     \
    SetMsgHandled(true);             \
    func();                          \
    *result = 0;                     \
    if (IsMsgHandled())              \
      return true;                   \
  }

#endif /* _WIN32_WINNT >= 0x0501 */

///////////////////////////////////////////////////////////////////////////////
// ATL defined messages

// BOOL OnForwardMsg(LPMSG Msg, DWORD nUserData)
#define CR_MSG_WM_FORWARDMSG(func)                         \
  if (uMsg == WM_FORWARDMSG) {                             \
    SetMsgHandled(true);                                   \
    *result = (LRESULT)func((LPMSG)lParam, (DWORD)wParam); \
    if (IsMsgHandled())                                    \
      return true;                                         \
  }

///////////////////////////////////////////////////////////////////////////////
// Dialog specific messages

// LRESULT OnDMGetDefID()
#define MSG_DM_GETDEFID(func) \
  if (uMsg == DM_GETDEFID) {  \
    SetMsgHandled(true);      \
    *result = func();         \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnDMSetDefID(UINT DefID)
#define MSG_DM_SETDEFID(func) \
  if (uMsg == DM_SETDEFID) {  \
    SetMsgHandled(true);      \
    func((UINT)wParam);       \
    *result = TRUE;           \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnDMReposition()
#define MSG_DM_REPOSITION(func) \
  if (uMsg == DM_REPOSITION) {  \
    SetMsgHandled(true);        \
    func();                     \
    *result = 0;                \
    if (IsMsgHandled())         \
      return true;              \
  }

///////////////////////////////////////////////////////////////////////////////
// Reflected messages

// void OnReflectedCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
#define MSG_OCM_COMMAND(func)                                      \
  if (uMsg == OCM_COMMAND) {                                       \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnReflectedNotify(int idCtrl, LPNMHDR pnmh)
#define MSG_OCM_NOTIFY(func)                      \
  if (uMsg == OCM_NOTIFY) {                       \
    SetMsgHandled(true);                          \
    *result = func((int)wParam, (LPNMHDR)lParam); \
    if (IsMsgHandled())                           \
      return true;                                \
  }

// void OnReflectedParentNotify(UINT message, UINT nChildID, LPARAM lParam)
#define MSG_OCM_PARENTNOTIFY(func)                            \
  if (uMsg == OCM_PARENTNOTIFY) {                             \
    SetMsgHandled(true);                                      \
    func((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), lParam); \
    *result = 0;                                              \
    if (IsMsgHandled())                                       \
      return true;                                            \
  }

// void OnReflectedDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
#define MSG_OCM_DRAWITEM(func)                    \
  if (uMsg == OCM_DRAWITEM) {                     \
    SetMsgHandled(true);                          \
    func((UINT)wParam, (LPDRAWITEMSTRUCT)lParam); \
    *result = TRUE;                               \
    if (IsMsgHandled())                           \
      return true;                                \
  }

// void OnReflectedMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT
// lpMeasureItemStruct)
#define MSG_OCM_MEASUREITEM(func)                    \
  if (uMsg == OCM_MEASUREITEM) {                     \
    SetMsgHandled(true);                             \
    func((UINT)wParam, (LPMEASUREITEMSTRUCT)lParam); \
    *result = TRUE;                                  \
    if (IsMsgHandled())                              \
      return true;                                   \
  }

// int OnReflectedCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT
// lpCompareItemStruct)
#define MSG_OCM_COMPAREITEM(func)                                       \
  if (uMsg == OCM_COMPAREITEM) {                                        \
    SetMsgHandled(true);                                                \
    *result = (LRESULT)func((UINT)wParam, (LPCOMPAREITEMSTRUCT)lParam); \
    if (IsMsgHandled())                                                 \
      return true;                                                      \
  }

// void OnReflectedDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct)
#define MSG_OCM_DELETEITEM(func)                    \
  if (uMsg == OCM_DELETEITEM) {                     \
    SetMsgHandled(true);                            \
    func((UINT)wParam, (LPDELETEITEMSTRUCT)lParam); \
    *result = TRUE;                                 \
    if (IsMsgHandled())                             \
      return true;                                  \
  }

// int OnReflectedVKeyToItem(UINT nKey, UINT nIndex, CListBox listBox)
#define MSG_OCM_VKEYTOITEM(func)                                   \
  if (uMsg == OCM_VKEYTOITEM) {                                    \
    SetMsgHandled(true);                                           \
    *result = (LRESULT)func(                                       \
        (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam); \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// int OnReflectedCharToItem(UINT nChar, UINT nIndex, CListBox listBox)
#define MSG_OCM_CHARTOITEM(func)                                   \
  if (uMsg == OCM_CHARTOITEM) {                                    \
    SetMsgHandled(true);                                           \
    *result = (LRESULT)func(                                       \
        (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam); \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// void OnReflectedHScroll(UINT nSBCode, UINT nPos, CScrollbar pScrollbar)
#define MSG_OCM_HSCROLL(func)                                       \
  if (uMsg == OCM_HSCROLL) {                                        \
    SetMsgHandled(true);                                            \
    func((int)LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// void OnReflectedVScroll(UINT nSBCode, UINT nPos, CScrollbar pScrollbar)
#define MSG_OCM_VSCROLL(func)                                       \
  if (uMsg == OCM_VSCROLL) {                                        \
    SetMsgHandled(true);                                            \
    func((int)LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); \
    *result = 0;                                                    \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// HBRUSH OnReflectedCtlColorEdit(CDCHandle dc, CEdit edit)
#define MSG_OCM_CTLCOLOREDIT(func)                      \
  if (uMsg == OCM_CTLCOLOREDIT) {                       \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnReflectedCtlColorListBox(CDCHandle dc, CListBox listBox)
#define MSG_OCM_CTLCOLORLISTBOX(func)                   \
  if (uMsg == OCM_CTLCOLORLISTBOX) {                    \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnReflectedCtlColorBtn(CDCHandle dc, CButton button)
#define MSG_OCM_CTLCOLORBTN(func)                       \
  if (uMsg == OCM_CTLCOLORBTN) {                        \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnReflectedCtlColorDlg(CDCHandle dc, CWindow wnd)
#define MSG_OCM_CTLCOLORDLG(func)                       \
  if (uMsg == OCM_CTLCOLORDLG) {                        \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnReflectedCtlColorScrollbar(CDCHandle dc, CScrollbar scrollBar)
#define MSG_OCM_CTLCOLORSCROLLBAR(func)                 \
  if (uMsg == OCM_CTLCOLORSCROLLBAR) {                  \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

// HBRUSH OnReflectedCtlColorStatic(CDCHandle dc, CStatic wndStatic)
#define MSG_OCM_CTLCOLORSTATIC(func)                    \
  if (uMsg == OCM_CTLCOLORSTATIC) {                     \
    SetMsgHandled(true);                                \
    *result = (LRESULT)func((HDC)wParam, (HWND)lParam); \
    if (IsMsgHandled())                                 \
      return true;                                      \
  }

///////////////////////////////////////////////////////////////////////////////
// Edit specific messages

// void OnClear()
#define CR_MSG_WM_CLEAR(func) \
  if (uMsg == WM_CLEAR) {     \
    SetMsgHandled(true);      \
    func();                   \
    *result = 0;              \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnCopy()
#define CR_MSG_WM_COPY(func) \
  if (uMsg == WM_COPY) {     \
    SetMsgHandled(true);     \
    func();                  \
    *result = 0;             \
    if (IsMsgHandled())      \
      return true;           \
  }

// void OnCut()
#define CR_MSG_WM_CUT(func) \
  if (uMsg == WM_CUT) {     \
    SetMsgHandled(true);    \
    func();                 \
    *result = 0;            \
    if (IsMsgHandled())     \
      return true;          \
  }

// void OnPaste()
#define CR_MSG_WM_PASTE(func) \
  if (uMsg == WM_PASTE) {     \
    SetMsgHandled(true);      \
    func();                   \
    *result = 0;              \
    if (IsMsgHandled())       \
      return true;            \
  }

// void OnUndo()
#define CR_MSG_WM_UNDO(func) \
  if (uMsg == WM_UNDO) {     \
    SetMsgHandled(true);     \
    func();                  \
    *result = 0;             \
    if (IsMsgHandled())      \
      return true;           \
  }

///////////////////////////////////////////////////////////////////////////////
// Generic message handlers

// LRESULT OnMessageHandlerEX(UINT uMsg, WPARAM wParam, LPARAM lParam)
#define CR_MESSAGE_HANDLER_EX(msg, func)  \
  if (uMsg == msg) {                      \
    SetMsgHandled(true);                  \
    *result = func(uMsg, wParam, lParam); \
    if (IsMsgHandled())                   \
      return true;                        \
  }

// LRESULT OnMessageRangeHandlerEX(UINT uMsg, WPARAM wParam, LPARAM lParam)
#define CR_MESSAGE_RANGE_HANDLER_EX(msgFirst, msgLast, func) \
  if (uMsg >= msgFirst && uMsg <= msgLast) {                 \
    SetMsgHandled(true);                                     \
    *result = func(uMsg, wParam, lParam);                    \
    if (IsMsgHandled())                                      \
      return true;                                           \
  }

///////////////////////////////////////////////////////////////////////////////
// Commands and notifications

// void OnCommandHandlerEX(UINT uNotifyCode, int nID, CWindow wndCtl)
#define CR_COMMAND_HANDLER_EX(id, code, func)                                 \
  if (uMsg == WM_COMMAND && code == HIWORD(wParam) && id == LOWORD(wParam)) { \
    SetMsgHandled(true);                                                      \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam);            \
    *result = 0;                                                              \
    if (IsMsgHandled())                                                       \
      return true;                                                            \
  }

// void OnCommandIDHandlerEX(UINT uNotifyCode, int nID, CWindow wndCtl)
#define CR_COMMAND_ID_HANDLER_EX(id, func)                         \
  if (uMsg == WM_COMMAND && id == LOWORD(wParam)) {                \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// void OnCommandCodeHandlerEX(UINT uNotifyCode, int nID, CWindow wndCtl)
#define CR_COMMAND_CODE_HANDLER_EX(code, func)                     \
  if (uMsg == WM_COMMAND && code == HIWORD(wParam)) {              \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnNotifyHandlerEX(LPNMHDR pnmh)
#define CR_NOTIFY_HANDLER_EX(id, cd, func)                  \
  if (uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code && \
      id == ((LPNMHDR)lParam)->idFrom) {                    \
    SetMsgHandled(true);                                    \
    *result = func((LPNMHDR)lParam);                        \
    if (IsMsgHandled())                                     \
      return true;                                          \
  }

// LRESULT OnNotifyIDHandlerEX(LPNMHDR pnmh)
#define CR_NOTIFY_ID_HANDLER_EX(id, func)                     \
  if (uMsg == WM_NOTIFY && id == ((LPNMHDR)lParam)->idFrom) { \
    SetMsgHandled(true);                                      \
    *result = func((LPNMHDR)lParam);                          \
    if (IsMsgHandled())                                       \
      return true;                                            \
  }

// LRESULT OnNotifyCodeHandlerEX(LPNMHDR pnmh)
#define CR_NOTIFY_CODE_HANDLER_EX(cd, func)                 \
  if (uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code) { \
    SetMsgHandled(true);                                    \
    *result = func((LPNMHDR)lParam);                        \
    if (IsMsgHandled())                                     \
      return true;                                          \
  }

// void OnCommandRangeHandlerEX(UINT uNotifyCode, int nID, CWindow wndCtl)
#define CR_COMMAND_RANGE_HANDLER_EX(idFirst, idLast, func)         \
  if (uMsg == WM_COMMAND && LOWORD(wParam) >= idFirst &&           \
      LOWORD(wParam) <= idLast) {                                  \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// void OnCommandRangeCodeHandlerEX(UINT uNotifyCode, int nID, CWindow wndCtl)
#define CR_COMMAND_RANGE_CODE_HANDLER_EX(idFirst, idLast, code, func) \
  if (uMsg == WM_COMMAND && code == HIWORD(wParam) &&                 \
      LOWORD(wParam) >= idFirst && LOWORD(wParam) <= idLast) {        \
    SetMsgHandled(true);                                              \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam);    \
    *result = 0;                                                      \
    if (IsMsgHandled())                                               \
      return true;                                                    \
  }

// LRESULT OnNotifyRangeHandlerEX(LPNMHDR pnmh)
#define CR_NOTIFY_RANGE_HANDLER_EX(idFirst, idLast, func)          \
  if (uMsg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom >= idFirst && \
      ((LPNMHDR)lParam)->idFrom <= idLast) {                       \
    SetMsgHandled(true);                                           \
    *result = func((LPNMHDR)lParam);                               \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnNotifyRangeCodeHandlerEX(LPNMHDR pnmh)
#define CR_NOTIFY_RANGE_CODE_HANDLER_EX(idFirst, idLast, cd, func) \
  if (uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code &&        \
      ((LPNMHDR)lParam)->idFrom >= idFirst &&                      \
      ((LPNMHDR)lParam)->idFrom <= idLast) {                       \
    SetMsgHandled(true);                                           \
    *result = func((LPNMHDR)lParam);                               \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnReflectedCommandHandlerEX(UINT uNotifyCode, int nID, CWindow
// wndCtl)
#define CR_REFLECTED_COMMAND_HANDLER_EX(id, code, func)                        \
  if (uMsg == OCM_COMMAND && code == HIWORD(wParam) && id == LOWORD(wParam)) { \
    SetMsgHandled(true);                                                       \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam);             \
    *result = 0;                                                               \
    if (IsMsgHandled())                                                        \
      return true;                                                             \
  }

// LRESULT OnReflectedCommandIDHandlerEX(UINT uNotifyCode, int nID, CWindow
// wndCtl)
#define CR_REFLECTED_COMMAND_ID_HANDLER_EX(id, func)               \
  if (uMsg == OCM_COMMAND && id == LOWORD(wParam)) {               \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnReflectedCommandCodeHandlerEX(UINT uNotifyCode, int nID, CWindow
// wndCtl)
#define CR_REFLECTED_COMMAND_CODE_HANDLER_EX(code, func)           \
  if (uMsg == OCM_COMMAND && code == HIWORD(wParam)) {             \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnReflectedNotifyHandlerEX(LPNMHDR pnmh)
#define CR_REFLECTED_NOTIFY_HANDLER_EX(id, cd, func)         \
  if (uMsg == OCM_NOTIFY && cd == ((LPNMHDR)lParam)->code && \
      id == ((LPNMHDR)lParam)->idFrom) {                     \
    SetMsgHandled(true);                                     \
    *result = func((LPNMHDR)lParam);                         \
    if (IsMsgHandled())                                      \
      return true;                                           \
  }

// LRESULT OnReflectedNotifyIDHandlerEX(LPNMHDR pnmh)
#define CR_REFLECTED_NOTIFY_ID_HANDLER_EX(id, func)            \
  if (uMsg == OCM_NOTIFY && id == ((LPNMHDR)lParam)->idFrom) { \
    SetMsgHandled(true);                                       \
    *result = func((LPNMHDR)lParam);                           \
    if (IsMsgHandled())                                        \
      return true;                                             \
  }

// LRESULT OnReflectedNotifyCodeHandlerEX(LPNMHDR pnmh)
#define CR_REFLECTED_NOTIFY_CODE_HANDLER_EX(cd, func)        \
  if (uMsg == OCM_NOTIFY && cd == ((LPNMHDR)lParam)->code) { \
    SetMsgHandled(true);                                     \
    *result = func((LPNMHDR)lParam);                         \
    if (IsMsgHandled())                                      \
      return true;                                           \
  }

// void OnReflectedCommandRangeHandlerEX(UINT uNotifyCode, int nID, CWindow
// wndCtl)
#define CR_REFLECTED_COMMAND_RANGE_HANDLER_EX(idFirst, idLast, func) \
  if (uMsg == OCM_COMMAND && LOWORD(wParam) >= idFirst &&            \
      LOWORD(wParam) <= idLast) {                                    \
    SetMsgHandled(true);                                             \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam);   \
    *result = 0;                                                     \
    if (IsMsgHandled())                                              \
      return true;                                                   \
  }

// void OnReflectedCommandRangeCodeHandlerEX(UINT uNotifyCode, int nID, CWindow
// wndCtl)
#define CR_REFLECTED_COMMAND_RANGE_CODE_HANDLER_EX(                \
    idFirst, idLast, code, func)                                   \
  if (uMsg == OCM_COMMAND && code == HIWORD(wParam) &&             \
      LOWORD(wParam) >= idFirst && LOWORD(wParam) <= idLast) {     \
    SetMsgHandled(true);                                           \
    func((UINT)HIWORD(wParam), (int)LOWORD(wParam), (HWND)lParam); \
    *result = 0;                                                   \
    if (IsMsgHandled())                                            \
      return true;                                                 \
  }

// LRESULT OnReflectedNotifyRangeHandlerEX(LPNMHDR pnmh)
#define CR_REFLECTED_NOTIFY_RANGE_HANDLER_EX(idFirst, idLast, func) \
  if (uMsg == OCM_NOTIFY && ((LPNMHDR)lParam)->idFrom >= idFirst && \
      ((LPNMHDR)lParam)->idFrom <= idLast) {                        \
    SetMsgHandled(true);                                            \
    *result = func((LPNMHDR)lParam);                                \
    if (IsMsgHandled())                                             \
      return true;                                                  \
  }

// LRESULT OnReflectedNotifyRangeCodeHandlerEX(LPNMHDR pnmh)
#define CR_REFLECTED_NOTIFY_RANGE_CODE_HANDLER_EX(idFirst, idLast, cd, func) \
  if (uMsg == OCM_NOTIFY && cd == ((LPNMHDR)lParam)->code &&                 \
      ((LPNMHDR)lParam)->idFrom >= idFirst &&                                \
      ((LPNMHDR)lParam)->idFrom <= idLast) {                                 \
    SetMsgHandled(true);                                                     \
    *result = func((LPNMHDR)lParam);                                         \
    if (IsMsgHandled())                                                      \
      return true;                                                           \
  }

#define CR_DEFLATE_RECT(rect, by)   \
  {                                 \
    (rect)->left += (by)->left;     \
    (rect)->top += (by)->top;       \
    (rect)->right -= (by)->right;   \
    (rect)->bottom -= (by)->bottom; \
  }

#define CR_POINT_INITIALIZER_FROM_LPARAM(lparam) \
  { LOWORD(lparam), HIWORD(lparam) }

#endif  // NATIVEUI_WIN_UTIL_MSG_UTIL_H_
