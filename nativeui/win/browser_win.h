// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_WIN_H_
#define NATIVEUI_WIN_BROWSER_WIN_H_

#include <exdisp.h>
#include <ole2.h>
#include <wrl.h>

#include "nativeui/browser.h"
#include "nativeui/win/browser_event_sink.h"
#include "nativeui/win/browser_ole_site.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

// Implementation of Browser.
class BrowserImpl : public SubwinView {
 public:
  explicit BrowserImpl(Browser* delegate);
  ~BrowserImpl() override;

  void LoadURL(const base::string16& str);

 protected:
  // ViewImpl:
  void SizeAllocate(const Rect& bounds) override;
  bool HasFocus() const override;

  CR_BEGIN_MSG_MAP_EX(BrowserImpl, SubwinView)
    CR_MSG_WM_DESTROY(OnDestroy)
    CR_MSG_WM_SETFOCUS(OnSetFocus)
    CR_MESSAGE_HANDLER_EX(WM_PARENTNOTIFY, OnParentNotify)
  CR_END_MSG_MAP()

 private:
  friend class BrowserEventSink;

  void OnDestroy();
  void OnSetFocus(HWND hwnd);
  LRESULT OnParentNotify(UINT msg, WPARAM w_param, LPARAM l_param);

  // Get the HWND of the IE control and add hooks.
  void ReceiveBrowserHWND();

  // Cleanup the hooks on IE control.
  void CleanupBrowserHWND();

  // The proc of IE control.
  static LRESULT CALLBACK BrowserWndProc(HWND hwnd,
                                         UINT message,
                                         WPARAM w_param,
                                         LPARAM l_param);

  Microsoft::WRL::ComPtr<IWebBrowser2> browser_;
  Microsoft::WRL::ComPtr<BrowserOleSite> ole_site_;
  Microsoft::WRL::ComPtr<BrowserEventSink> event_sink_;
  HWND browser_hwnd_ = NULL;
  WNDPROC browser_proc_ = nullptr;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_WIN_H_
