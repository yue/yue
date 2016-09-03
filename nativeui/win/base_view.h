// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BASE_VIEW_H_
#define NATIVEUI_WIN_BASE_VIEW_H_

#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/win/window_impl.h"

namespace nu {

class WindowImpl;

// The common base for native window based view and directui view.
class BaseView {
 public:
  virtual ~BaseView() {}

  // Subclass should override this.
  virtual void SetPixelBounds(const Rect& pixel_bounds);
  virtual Rect GetPixelBounds() const;

  // Draw the content.
  virtual void Draw(Gdiplus::Graphics* context, const Rect& dirty) {}

  // Set the parent view.
  virtual void SetParent(BaseView* parent);
  virtual void BecomeContentView(WindowImpl* parent);

  // Get the offset to parent HWND.
  Point GetWindowPixelOrigin();
  Rect GetWindowPixelBounds();

  // Returns DIP bounds according to window's scale factor.
  void SetBounds(const Rect& bounds);
  Rect GetBounds();

  // Invalidate the view and trigger a redraw.
  void Invalidate(const Rect& dirty = Rect());

  // Set the preferred size.
  void set_pixel_preferred_size(Size size) { preferred_size_ = size; }
  Size pixel_preferred_size() const { return preferred_size_; }

  // Whether the view is visible.
  void set_visible(bool visible) { is_visible_ = visible; }
  bool is_visible() const { return is_visible_; }

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

  // The visible state.
  bool is_visible_ = true;

  bool is_content_view_ = false;
  WindowImpl* window_ = nullptr;
  BaseView* parent_ = nullptr;

  // The bounds relative to parent view.
  Rect bounds_;

  // The preferred size of the view.
  Size preferred_size_;

  // The offset relative the parent HWND.
  Point window_origin_;

  DISALLOW_COPY_AND_ASSIGN(BaseView);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BASE_VIEW_H_
