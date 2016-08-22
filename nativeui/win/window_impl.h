// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_WINDOW_IMPL_H_
#define NATIVEUI_WIN_WINDOW_IMPL_H_

#include <windows.h>

#include <string>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/win/msg_util.h"

namespace nu {

// A convenience class that encapsulates the details of creating and destroying
// a HWND. This class also hosts the windows procedure used by all Windows.
class WindowImpl {
 public:
  virtual ~WindowImpl();

  // Returns the HWND associated with this Window.
  HWND hwnd() const { return hwnd_; }

 protected:
  static const DWORD kWindowDefaultChildStyle;
  static const DWORD kWindowDefaultStyle;

  WindowImpl(base::StringPiece16 class_name = L"",
             HWND parent = NULL,
             DWORD window_style = kWindowDefaultStyle,
             DWORD window_ex_style = 0);

  // Returns the default window icon to use for windows of this type.
  virtual HICON GetDefaultWindowIcon() const;
  virtual HICON GetSmallWindowIcon() const;

  // Handles the WndProc callback for this object.
  virtual LRESULT OnWndProc(UINT message, WPARAM w_param, LPARAM l_param);

  // Processes one message from the window's message queue.
  virtual BOOL ProcessWindowMessage(HWND window,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param,
                                    LRESULT& result,  // NOLINT
                                    DWORD msg_map_id = 0);

 private:
  friend class ClassRegistrar;

  // The window procedure used by all Windows.
  static LRESULT CALLBACK WndProc(HWND window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param);

  // Gets the window class atom to use when creating the corresponding HWND.
  // If necessary, this registers the window class.
  ATOM GetWindowClassAtom();

  // Style of the class to use.
  UINT class_style_;

  // Our hwnd.
  HWND hwnd_ = NULL;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_WINDOW_IMPL_H_
