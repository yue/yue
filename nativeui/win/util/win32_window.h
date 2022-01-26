// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_WIN32_WINDOW_H_
#define NATIVEUI_WIN_UTIL_WIN32_WINDOW_H_

#include <windows.h>

#include <string>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/win/util/msg_util.h"

namespace nu {

// A convenience class that encapsulates the details of creating and destroying
// a HWND. This class also hosts the windows procedure used by all Windows.
class Win32Window {
 public:
  virtual ~Win32Window();

  void SetWindowStyle(LONG style, bool on);
  bool HasWindowStyle(LONG style) const;
  void ExecuteSystemMenuCommand(int command);

  // Returns the HWND associated with this Window.
  HWND hwnd() const { return hwnd_; }

  // Window styles.
  DWORD window_style() const { return window_style_; }
  DWORD window_ex_style() const { return window_ex_style_; }

 protected:
  static const DWORD kWindowDefaultChildStyle;
  static const DWORD kWindowDefaultStyle;

  Win32Window(base::WStringPiece class_name = L"",
              HWND parent = NULL,
              DWORD window_style = kWindowDefaultStyle,
              DWORD window_ex_style = 0);

  // Return the default window icon to use for windows of this type.
  virtual HICON GetDefaultWindowIcon() const;
  virtual HICON GetSmallWindowIcon() const;

  // Handle the WndProc callback for this object.
  virtual LRESULT OnWndProc(UINT message, WPARAM w_param, LPARAM l_param);

  // Process one message from the window's message queue.
  virtual bool ProcessWindowMessage(HWND window,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param,
                                    LRESULT* result);

 private:
  friend class ClassRegistrar;

  // The window procedure used by all Windows.
  static LRESULT CALLBACK WndProc(HWND window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param);

  // Get the window class atom to use when creating the corresponding HWND.
  // If necessary, this registers the window class.
  ATOM GetWindowClassAtom();

  // The window handle.
  HWND hwnd_ = NULL;

  // Window styles.
  DWORD window_style_;
  DWORD window_ex_style_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_WIN32_WINDOW_H_
