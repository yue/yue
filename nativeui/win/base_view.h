// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BASE_VIEW_H_
#define NATIVEUI_WIN_BASE_VIEW_H_

#include <windows.h>

#include "base/macros.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

// The common base for windows.
class BaseWindow {
 public:
  virtual ~BaseWindow() {}

  HWND hwnd() const { return hwnd_; }
  float scale_factor() const { return scale_factor_; }

 protected:
  explicit BaseWindow(HWND hwnd);

 private:
  HWND hwnd_;

  // The scale factor of current window.
  // TODO(zcbenz): Refresh the window when DPI changes.
  float scale_factor_;

  DISALLOW_COPY_AND_ASSIGN(BaseWindow);
};

// The common base for native window based view and directui view.
class BaseView {
 public:
  virtual ~BaseView() {}

  // Subclass should override this.
  virtual void SetPixelBounds(const gfx::Rect& pixel_bounds);
  virtual gfx::Rect GetPixelBounds();

  // Set the parent view.
  virtual void SetParent(BaseView* parent);
  virtual void BecomeContentView(BaseWindow* parent);

  // Returns DIP bounds according to window's scale factor.
  void SetBounds(const gfx::Rect& bounds);
  gfx::Rect GetBounds();

  // Parent view and host window.
  BaseWindow* window() const { return window_; }
  BaseView* parent() const { return parent_; }

  // Whether it is the content view.
  bool is_content_view() const { return is_content_view_; }

  // Whether the view owns a HWND.
  bool is_virtual() const { return is_virtual_; }

  // Returns the DPI of current view.
  float scale_factor() const { return window_ ? window_->scale_factor() : 1.f; }

 protected:
  explicit BaseView(bool is_virtual) : is_virtual_(is_virtual) {}

 private:
  bool is_virtual_;

  bool is_content_view_ = false;
  BaseWindow* window_ = nullptr;
  BaseView* parent_ = nullptr;

  // The bounds relative to parent.
  gfx::Rect bounds_;

  DISALLOW_COPY_AND_ASSIGN(BaseView);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BASE_VIEW_H_
