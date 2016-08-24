// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BASE_VIEW_H_
#define NATIVEUI_WIN_BASE_VIEW_H_

#include "nativeui/win/gdiplus.h"
#include "nativeui/win/window_impl.h"

namespace nu {

class WindowImpl;

// The common base for native window based view and directui view.
class BaseView {
 public:
  virtual ~BaseView() {}

  // Subclass should override this.
  virtual void SetPixelBounds(const gfx::Rect& pixel_bounds);
  virtual gfx::Rect GetPixelBounds();

  // Draw the content.
  virtual void Draw(Gdiplus::Graphics* context, const gfx::Rect& dirty) {}

  // Set the parent view.
  virtual void SetParent(BaseView* parent);
  virtual void BecomeContentView(WindowImpl* parent);

  // Get the offset to parent HWND.
  gfx::Point GetWindowPixelOrigin();

  // Returns DIP bounds according to window's scale factor.
  void SetBounds(const gfx::Rect& bounds);
  gfx::Rect GetBounds();

  // Parent view and host window.
  WindowImpl* window() const { return window_; }
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
  WindowImpl* window_ = nullptr;
  BaseView* parent_ = nullptr;

  // The bounds relative to parent view.
  gfx::Rect bounds_;

  // The offset relative the parent HWND.
  gfx::Point window_origin_;

  DISALLOW_COPY_AND_ASSIGN(BaseView);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BASE_VIEW_H_
