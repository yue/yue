// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_UTIL_HWND_UTIL_H_
#define NATIVEUI_WIN_UTIL_HWND_UTIL_H_

#include <windows.h>

#include <string>

#include "nativeui/nativeui_export.h"

namespace nu {

class Point;
class Size;

// A version of the GetClassNameW API that returns the class name in an
// std::wstring. An empty result indicates a failure to get the class name.
std::wstring GetClassName(HWND hwnd);

// Useful for subclassing a HWND.  Returns the previous window procedure.
WNDPROC SetWindowProc(HWND hwnd, WNDPROC wndproc);

// Pointer-friendly wrappers around Get/SetWindowLong(..., GWLP_USERDATA, ...)
// Returns the previously set value.
void* SetWindowUserData(HWND hwnd, void* user_data);
void* GetWindowUserData(HWND hwnd);

// Returns true if the specified window is the current active top window or one
// of its children.
bool DoesWindowBelongToActiveWindow(HWND window);

// Sizes the window to have a window size of |pref|, then centers the window
// over |parent|, ensuring the window fits on screen.
void CenterAndSizeWindow(HWND parent,
                                         HWND window,
                                         const Size& pref);

// If |hwnd| is NULL logs various thing and CHECKs. Invoke right after calling
// CreateWindow.
void CheckWindowCreated(HWND hwnd);

// Returns the window you can use to parent a top level window.
// Note that in some cases we create child windows not parented to its final
// container so in those cases you should pass true in |get_real_hwnd|.
HWND GetWindowToParentTo(bool get_real_hwnd);

// Get the window text.
std::wstring GetWindowString(HWND hwnd);

// The size, in pixels, of the non-client frame around a window.
int GetFrameThickness(float scale_factor);

// Returns whether the edge has auto-hide taskbar.
bool MonitorHasAutohideTaskbarForEdge(UINT edge, HMONITOR monitor);

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_HWND_UTIL_H_
