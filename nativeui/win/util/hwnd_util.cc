// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/util/hwnd_util.h"

#include <shellapi.h>

#include "base/logging.h"
#include "base/notreached.h"
#include "base/strings/string_util.h"
#include "base/win/win_util.h"
#include "nativeui/gfx/geometry/point.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/geometry/size.h"
#include "nativeui/win/screen_win.h"

namespace nu {

namespace {

// Adjust the window to fit.
void AdjustWindowToFit(HWND hwnd, const RECT& bounds, bool fit_to_monitor) {
  if (fit_to_monitor) {
    // Get the monitor.
    HMONITOR hmon = MonitorFromRect(&bounds, MONITOR_DEFAULTTONEAREST);
    if (hmon) {
      MONITORINFO mi;
      mi.cbSize = sizeof(mi);
      GetMonitorInfo(hmon, &mi);
      Rect window_rect(bounds);
      Rect monitor_rect(mi.rcWork);
      Rect new_window_rect = window_rect;
      new_window_rect.AdjustToFit(monitor_rect);
      if (new_window_rect != window_rect) {
        // Window doesn't fit on monitor, move and possibly resize.
        SetWindowPos(hwnd, 0, new_window_rect.x(), new_window_rect.y(),
                     new_window_rect.width(), new_window_rect.height(),
                     SWP_NOACTIVATE | SWP_NOZORDER);
        return;
      }
      // Else fall through.
    } else {
      NOTREACHED() << "Unable to find default monitor";
      // Fall through.
    }
  }  // Else fall through.

  // The window is not being fit to monitor, or the window fits on the monitor
  // as is, or we have no monitor info; reset the bounds.
  ::SetWindowPos(hwnd, 0, bounds.left, bounds.top,
                 bounds.right - bounds.left, bounds.bottom - bounds.top,
                 SWP_NOACTIVATE | SWP_NOZORDER);
}

// Turn off optimizations for these functions so they show up in crash reports.
#if defined(COMPILER_MSVC)
__pragma(optimize("", off))
#endif

void CrashOutOfMemory() {
  PLOG(FATAL);
}

void CrashAccessDenied() {
  PLOG(FATAL);
}

// Crash isn't one of the ones we commonly see.
void CrashOther() {
  PLOG(FATAL);
}

#if defined(COMPILER_MSVC)
__pragma(optimize("", on))
#endif

}  // namespace

std::wstring GetClassName(HWND window) {
  // GetClassNameW will return a truncated result (properly null terminated) if
  // the given buffer is not large enough.  So, it is not possible to determine
  // that we got the entire class name if the result is exactly equal to the
  // size of the buffer minus one.
  DWORD buffer_size = MAX_PATH;
  while (true) {
    std::wstring output;
    DWORD size_ret = GetClassNameW(
        window, base::WriteInto(&output, buffer_size), buffer_size);
    if (size_ret == 0)
      break;
    if (size_ret < (buffer_size - 1)) {
      output.resize(size_ret);
      return output;
    }
    buffer_size *= 2;
  }
  return std::wstring();  // error
}

#pragma warning(push)
#pragma warning(disable:4312 4244)

WNDPROC SetWindowProc(HWND hwnd, WNDPROC proc) {
  // The reason we don't return the SetwindowLongPtr() value is that it returns
  // the orignal window procedure and not the current one. I don't know if it is
  // a bug or an intended feature.
  WNDPROC oldwindow_proc =
      reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC));
  SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc));
  return oldwindow_proc;
}

void* SetWindowUserData(HWND hwnd, void* user_data) {
  return
      reinterpret_cast<void*>(SetWindowLongPtr(hwnd, GWLP_USERDATA,
          reinterpret_cast<LONG_PTR>(user_data)));
}

void* GetWindowUserData(HWND hwnd) {
  DWORD process_id = 0;
  GetWindowThreadProcessId(hwnd, &process_id);
  // A window outside the current process needs to be ignored.
  if (process_id != ::GetCurrentProcessId())
    return NULL;
  return reinterpret_cast<void*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

#pragma warning(pop)

bool DoesWindowBelongToActiveWindow(HWND window) {
  DCHECK(window);
  HWND top_window = ::GetAncestor(window, GA_ROOT);
  if (!top_window)
    return false;

  HWND active_top_window = ::GetAncestor(::GetForegroundWindow(), GA_ROOT);
  return (top_window == active_top_window);
}

void CenterAndSizeWindow(HWND parent,
                         HWND window,
                         const Size& pref) {
  DCHECK(window && pref.width() > 0 && pref.height() > 0);

  // Calculate the ideal bounds.
  RECT window_bounds;
  RECT center_bounds = {0};
  if (parent) {
    // If there is a parent, center over the parents bounds.
    ::GetWindowRect(parent, &center_bounds);
  }

  if (::IsRectEmpty(&center_bounds)) {
    // No parent or no parent rect. Center over the monitor the window is on.
    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
    if (monitor) {
      MONITORINFO mi = {0};
      mi.cbSize = sizeof(mi);
      GetMonitorInfo(monitor, &mi);
      center_bounds = mi.rcWork;
    } else {
      NOTREACHED() << "Unable to get default monitor";
    }
  }

  window_bounds.left = center_bounds.left;
  if (pref.width() < (center_bounds.right - center_bounds.left)) {
    window_bounds.left +=
        (center_bounds.right - center_bounds.left - pref.width()) / 2;
  }
  window_bounds.right = window_bounds.left + pref.width();

  window_bounds.top = center_bounds.top;
  if (pref.height() < (center_bounds.bottom - center_bounds.top)) {
    window_bounds.top +=
        (center_bounds.bottom - center_bounds.top - pref.height()) / 2;
  }
  window_bounds.bottom = window_bounds.top + pref.height();

  // If we're centering a child window, we are positioning in client
  // coordinates, and as such we need to offset the target rectangle by the
  // position of the parent window.
  if (::GetWindowLong(window, GWL_STYLE) & WS_CHILD) {
    DCHECK(parent && ::GetParent(window) == parent);
    POINT topleft = { window_bounds.left, window_bounds.top };
    ::MapWindowPoints(HWND_DESKTOP, parent, &topleft, 1);
    window_bounds.left = topleft.x;
    window_bounds.top = topleft.y;
    window_bounds.right = window_bounds.left + pref.width();
    window_bounds.bottom = window_bounds.top + pref.height();
  }

  AdjustWindowToFit(window, window_bounds, !parent);
}

void CheckWindowCreated(HWND hwnd) {
  if (!hwnd) {
    switch (GetLastError()) {
      case ERROR_NOT_ENOUGH_MEMORY:
        CrashOutOfMemory();
        break;
      case ERROR_ACCESS_DENIED:
        CrashAccessDenied();
        break;
      default:
        CrashOther();
        break;
    }
    PLOG(FATAL);
  }
}

HWND GetWindowToParentTo(bool get_real_hwnd) {
  return get_real_hwnd ? ::GetDesktopWindow() : HWND_DESKTOP;
}

std::wstring GetWindowString(HWND hwnd) {
  std::wstring title;
  int len = ::GetWindowTextLengthW(hwnd) + 1;
  if (len > 1)
    ::GetWindowTextW(hwnd, base::WriteInto(&title, len), len);
  return title;
}

int GetFrameThickness(float scale_factor) {
  // On Windows 10 the visible frame border is one pixel thick, but there is
  // some additional non-visible space: SM_CXSIZEFRAME (the resize handle)
  // and SM_CXPADDEDBORDER (additional border space that isn't part of the
  // resize handle).
  const int resize_frame_thickness =
      GetSystemMetricsForScaleFactor(scale_factor, SM_CXSIZEFRAME);
  const int padding_thickness =
      GetSystemMetricsForScaleFactor(scale_factor, SM_CXPADDEDBORDER);
  return resize_frame_thickness + padding_thickness;
}

bool MonitorHasAutohideTaskbarForEdge(UINT edge, HMONITOR monitor) {
  APPBARDATA taskbar_data = {sizeof(APPBARDATA), NULL, 0, edge};
  taskbar_data.hWnd = ::GetForegroundWindow();

  // MSDN documents an ABM_GETAUTOHIDEBAREX, which supposedly takes a monitor
  // rect and returns autohide bars on that monitor.  This sounds like a good
  // idea for multi-monitor systems.  Unfortunately, it appears to not work at
  // least some of the time (erroneously returning NULL) and there's almost no
  // online documentation or other sample code using it that suggests ways to
  // address this problem. We do the following:-
  // 1. Use the ABM_GETAUTOHIDEBAR message. If it works, i.e. returns a valid
  //    window we are done.
  // 2. If the ABM_GETAUTOHIDEBAR message does not work we query the auto hide
  //    state of the taskbar and then retrieve its position. That call returns
  //    the edge on which the taskbar is present. If it matches the edge we
  //    are looking for, we are done.
  // NOTE: This call spins a nested run loop.
  HWND taskbar = reinterpret_cast<HWND>(
      SHAppBarMessage(ABM_GETAUTOHIDEBAR, &taskbar_data));
  if (!::IsWindow(taskbar)) {
    APPBARDATA taskbar_data = {sizeof(APPBARDATA), 0, 0, 0};
    unsigned int taskbar_state = SHAppBarMessage(ABM_GETSTATE, &taskbar_data);
    if (!(taskbar_state & ABS_AUTOHIDE))
      return false;

    taskbar_data.hWnd = ::FindWindow(L"Shell_TrayWnd", NULL);
    if (!::IsWindow(taskbar_data.hWnd))
      return false;

    SHAppBarMessage(ABM_GETTASKBARPOS, &taskbar_data);
    if (taskbar_data.uEdge == edge)
      taskbar = taskbar_data.hWnd;
  }

  // There is a potential race condition here:
  // 1. A maximized chrome window is fullscreened.
  // 2. It is switched back to maximized.
  // 3. In the process the window gets a WM_NCCACLSIZE message which calls us to
  //    get the autohide state.
  // 4. The worker thread is invoked. It calls the API to get the autohide
  //    state. On Windows versions  earlier than Windows 7, taskbars could
  //    easily be always on top or not.
  //    This meant that we only want to look for taskbars which have the topmost
  //    bit set.  However this causes problems in cases where the window on the
  //    main thread is still in the process of switching away from fullscreen.
  //    In this case the taskbar might not yet have the topmost bit set.
  // 5. The main thread resumes and does not leave space for the taskbar and
  //    hence it does not pop when hovered.
  //
  // To address point 4 above, it is best to not check for the WS_EX_TOPMOST
  // window style on the taskbar, as starting from Windows 7, the topmost
  // style is always set. We don't support XP and Vista anymore.
  if (::IsWindow(taskbar)) {
    if (MonitorFromWindow(taskbar, MONITOR_DEFAULTTONEAREST) == monitor)
      return true;
    // In some cases like when the autohide taskbar is on the left of the
    // secondary monitor, the MonitorFromWindow call above fails to return the
    // correct monitor the taskbar is on. We fallback to MonitorFromPoint for
    // the cursor position in that case, which seems to work well.
    POINT cursor_pos = {0};
    GetCursorPos(&cursor_pos);
    if (MonitorFromPoint(cursor_pos, MONITOR_DEFAULTTONEAREST) == monitor)
      return true;
  }
  return false;
}

}  // namespace nu
