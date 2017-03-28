// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BASE_VIEW_H_
#define NATIVEUI_WIN_BASE_VIEW_H_

#include "nativeui/gfx/win/painter_win.h"
#include "nativeui/win/screen.h"
#include "nativeui/win/window_win.h"

namespace nu {

class ScrollView;
class WindowImpl;

// The state of the control.
enum class ControlState {
  // IDs defined as specific values for use in arrays.
  Disabled = 0,
  Hovered  = 1,
  Normal   = 2,
  Pressed  = 3,
  Size     = Pressed + 1,
};

// Possible control types.
enum class ControlType {
  Button,
  CheckBox,
  Radio,
  Container,
  Group,
  Label,
  Scroll,
  ScrollBar,
  ScrollBarButton,
  ScrollBarThumb,
  Subwin,
};

// The common base for native window based view and directui view.
class BaseView {
 public:
  virtual ~BaseView() {}

  /////////////////////////////////////////////////////////////////////////////
  // Core implementations, should be overriden for each kind of view

  // Changes view's bounds, relative to window.
  virtual void SizeAllocate(const Rect& bounds);

  // Set the parent view.
  virtual void SetParent(BaseView* parent);
  virtual void BecomeContentView(WindowImpl* parent);

  // Invalidate the |dirty| rect.
  virtual void Invalidate(const Rect& dirty);

  // Whether the view can get focus.
  virtual bool CanHaveFocus() const { return false; }

  // Move focus to the view.
  virtual void SetFocus(bool focus);
  virtual bool IsFocused() const;

  // Set the background color.
  virtual void SetBackgroundColor(Color color);

  /////////////////////////////////////////////////////////////////////////////
  // Events

  // Draw the content.
  virtual void Draw(PainterWin* painter, const Rect& dirty);

  // The mouse events.
  virtual void OnMouseEnter() {}
  virtual void OnMouseMove(UINT flags, const Point& point) {}
  virtual void OnMouseLeave() {}
  virtual bool OnMouseWheel(bool vertical, UINT flags, int delta,
                            const Point& point) { return false; }
  virtual bool OnMouseClick(UINT message, UINT flags, const Point& point) {
      return false;
  }

  // Called when the view lost capture.
  virtual void OnCaptureLost() {}

  /////////////////////////////////////////////////////////////////////////////
  // Helpers

  // Get the mouse position in current view.
  Point GetMousePosition() const;

  // Get the size allocation that inside viewport.
  Rect GetClippedRect() const;

  // Invalidate the whole view.
  void Invalidate();

  // Change the bounds without invalidating.
  void set_size_allocation(const Rect& bounds) { size_allocation_ = bounds; }
  Rect size_allocation() const { return size_allocation_; }

  // Set the preferred size.
  void set_preferred_size(const Size& size) { preferred_size_ = size; }
  Size preferred_size() const { return preferred_size_; }

  // Whether the view is visible.
  void set_visible(bool visible) { is_visible_ = visible; }
  bool is_visible() const { return is_visible_; }

  // Set control's state.
  void set_state(ControlState state) { state_ = state; }
  ControlState state() const { return state_; }

  // Returns the DPI of current view.
  float scale_factor() const {
    if (window_)
      return window_->scale_factor();
    static float current_scale_factor = GetDPIScale();
    return current_scale_factor;
  }

  Color background_color() const { return background_color_; }

  WindowImpl* window() const { return window_; }
  ControlType type() const { return type_; }

 protected:
  explicit BaseView(ControlType type) : type_(type) {}

  // Called by SetParent/BecomeContentView when parent view changes.
  void ParentChanged(float old_scale_factor);

 private:
  ControlType type_;

  // The background color.
  Color background_color_ = Color(0, 0, 0, 0);  // transparent

  // The focus state.
  bool is_focused_ = false;

  // The visible state.
  bool is_visible_ = true;

  // The control state.
  ControlState state_ = ControlState::Normal;

  // The window that owns the view.
  WindowImpl* window_ = nullptr;

  // The viewport that owns this view.
  ScrollView* viewport_ = nullptr;

  // The absolute bounds relative to the origin of window.
  Rect size_allocation_;

  // The preferred size of the view.
  Size preferred_size_;

  DISALLOW_COPY_AND_ASSIGN(BaseView);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BASE_VIEW_H_
