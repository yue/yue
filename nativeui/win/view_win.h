// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_VIEW_WIN_H_
#define NATIVEUI_WIN_VIEW_WIN_H_

#include <set>

#include "nativeui/cursor.h"
#include "nativeui/gfx/win/painter_win.h"
#include "nativeui/view.h"
#include "nativeui/win/window_win.h"

namespace nu {

class ScrollImpl;
class WindowImpl;

// Possible control types.
// We only add new types when it is necessary, otherwise a new View should
// usually just be |Container| or |View|.
enum class ControlType {
  View,
  Subwin,
  Scrollbar,
  // Button types.
  Button,
  Checkbox,
  Radio,
  // Container types, other types should NOT be appended after Container.
  Container,
  Scroll,
};

// The common base for native window based view and directui view.
class ViewImpl {
 public:
  virtual ~ViewImpl();

  ViewImpl& operator=(const ViewImpl&) = delete;
  ViewImpl(const ViewImpl&) = delete;

  /////////////////////////////////////////////////////////////////////////////
  // Core implementations, should be overriden for each kind of view

  // Change view's bounds, relative to window.
  virtual void SizeAllocate(const Rect& bounds);

  // Return the hittest result for pixel |point|.
  virtual UINT HitTest(const Point& point) const;

  // Set the parent view.
  virtual void SetParent(ViewImpl* parent);
  virtual void BecomeContentView(WindowImpl* parent);

  // Invalidate the |dirty| rect.
  virtual void Invalidate(const Rect& dirty);

  // Calculate the clipped rect of a child.
  virtual void ClipRectForChild(const ViewImpl* child, Rect* rect) const;

  // Move focus to the view.
  virtual void SetFocus(bool focus);
  virtual bool HasFocus() const;

  // This view or parent view has changed its visibility.
  virtual void VisibilityChanged();

  // Set styles.
  virtual void SetFont(Font* font);
  virtual void SetColor(Color color);
  virtual void SetBackgroundColor(Color color);

  // Set control's state.
  virtual void SetState(ControlState state);

  /////////////////////////////////////////////////////////////////////////////
  // Common APIs.

  // Drag and drop.
  void RegisterDraggedTypes(std::set<Clipboard::Data::Type> types);

  /////////////////////////////////////////////////////////////////////////////
  // Events

  // Draw the content.
  virtual void Draw(PainterWin* painter, const Rect& dirty);

  // The DPI of this view has changed.
  virtual void OnDPIChanged() {}

  // The input events.
  virtual void OnMouseMove(NativeEvent event);
  virtual void OnMouseEnter(NativeEvent event);
  virtual void OnMouseLeave(NativeEvent event);
  virtual bool OnMouseWheel(NativeEvent event);
  virtual bool OnMouseClick(NativeEvent event);
  virtual bool OnSetCursor(NativeEvent event);
  virtual bool OnKeyEvent(NativeEvent event);

  // Called when the view lost capture.
  virtual void OnCaptureLost();

  /////////////////////////////////////////////////////////////////////////////
  // Drag and drop

  // Drop target events.
  virtual int OnDragEnter(IDataObject* data, int effect, const Point& point);
  virtual int OnDragUpdate(IDataObject* data, int effect, const Point& point);
  virtual void OnDragLeave(IDataObject* data);
  virtual int OnDrop(IDataObject* data, int effect, const Point& point);

  /////////////////////////////////////////////////////////////////////////////
  // Helpers

  // whether the view accepts dropping.
  bool AcceptsDropping(IDataObject* data);

  // Get the mouse position in current view.
  Point GetMousePosition() const;

  // Get the size allocation that inside viewport.
  Rect GetClippedRect() const;

  // Show/Hide the view.
  void SetVisible(bool visible);

  // Invalidate the whole view.
  void Invalidate();

  // Change the bounds without invalidating.
  void set_size_allocation(const Rect& bounds) { size_allocation_ = bounds; }
  Rect size_allocation() const { return size_allocation_; }

  // Whether the view is visible.
  bool is_visible() const { return is_visible_; }

  // Whether the view and its parent are visible.
  bool is_tree_visible() const { return is_tree_visible_; }

  // Whether the view is enabled.
  void set_enabled(bool b) { is_enabled_ = b; }
  bool is_enabled() const { return is_enabled_; }

  // Whether the view can get focus.
  void set_focusable(bool focusable) { focusable_ = focusable; }
  bool is_focusable() const { return is_enabled() && focusable_; }

  void set_draggable(bool draggable) { draggable_ = draggable; }
  bool is_draggable() const { return draggable_; }

  // Set control's viewport, only called by Scroll.
  void set_viewport(ScrollImpl* scroll) { viewport_ = scroll; }

  // Returns the DPI of current view.
  float scale_factor() const { return scale_factor_; }

  Cursor* cursor() const { return delegate_ ? delegate_->cursor() : nullptr; }
  Font* font() const { return font_; }
  Color color() const { return color_; }
  Color background_color() const { return background_color_; }
  ControlState state() const { return state_; }

  WindowImpl* window() const { return window_; }
  ViewImpl* parent() const { return parent_; }
  ControlType type() const { return type_; }
  View* delegate() const { return delegate_; }

 protected:
  ViewImpl(ControlType type, View* delegate);

  // Called by SetParent/BecomeContentView when parent view changes.
  void ParentChanged();

 private:
  ControlType type_;

  // Whether the view can have focus.
  bool focusable_ = false;

  // Whether dragging the view can move the window.
  bool draggable_ = false;

  // Styles.
  Font* font_;  // weak ptr
  Color color_;
  Color background_color_ = Color(0, 0, 0, 0);  // transparent

  // The focus state.
  bool is_focused_ = false;

  // The visible state.
  bool is_visible_ = true;

  // Whether the view and its parent are visible.
  bool is_tree_visible_ = true;

  // The enabled state.
  bool is_enabled_ = true;

  // Accepted dragged types.
  std::set<Clipboard::Data::Type> dragged_types_;

  // The last returned drag operation.
  int last_drag_effect_ = DRAG_OPERATION_UNHANDLED;

  // The control state.
  ControlState state_ = ControlState::Normal;

  // The window that owns the view.
  WindowImpl* window_ = nullptr;

  // The parent view. Weak ref.
  ViewImpl* parent_ = nullptr;

  // The viewport that owns this view.
  ScrollImpl* viewport_ = nullptr;

  // The scale factor this view uses.
  float scale_factor_;

  // The view which this class implements, this can be nullptr;
  View* delegate_;

  // The absolute bounds relative to the origin of window.
  Rect size_allocation_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_VIEW_WIN_H_
