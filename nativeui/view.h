// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include <set>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/clipboard.h"
#include "nativeui/dragging_info.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/responder.h"

typedef struct YGNode *YGNodeRef;
typedef struct YGConfig *YGConfigRef;

namespace nu {

class Cursor;
class Font;
class Popover;
class Window;

// The base class for all kinds of views.
class NATIVEUI_EXPORT View : public base::RefCounted<View>,
                             public Responder<View> {
 public:
  // The view class name.
  static const char kClassName[];

  // Return the receiving view's class name. A view class is a string which
  // uniquely identifies the view class. It is intended to be used as a way to
  // find out during run time if a view can be safely casted to a specific view
  // subclass.
  virtual const char* GetClassName() const;

  // Coordiante convertions.
  Vector2dF OffsetFromView(const View* from) const;
  Vector2dF OffsetFromWindow() const;

  // Internal: Change position and size.
  void SetBounds(const RectF& bounds);

  // Get position and size.
  RectF GetBounds() const;
  RectF GetBoundsInWindow() const;

  // Internal: The real pixel bounds that depends on the scale factor.
  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds() const;

  // Update layout.
  virtual void Layout();

  // Mark the whole view as dirty.
  void SchedulePaint();

  // Repaint the rect
  void SchedulePaintRect(const RectF& rect);

  // Show/Hide the view.
  void SetVisible(bool visible);
  bool IsVisible() const;

  // Whether the view and its parent are visible.
  // TODO(zcbenz): Find a better name before making it public.
  bool IsTreeVisible() const;

  // Enable/disable the view.
  void SetEnabled(bool enable);
  bool IsEnabled() const;

  // Move the keyboard focus to the view.
  void Focus();
  bool HasFocus() const;

  // Whether the view can be focused.
  void SetFocusable(bool focusable);
  bool IsFocusable() const;

  // Capture mouse.
  void SetCapture();
  void ReleaseCapture();
  bool HasCapture() const;

  // Dragging the view would move the window.
  void SetMouseDownCanMoveWindow(bool yes);
  bool IsMouseDownCanMoveWindow() const;

  // Drag and drop.
  int DoDrag(std::vector<Clipboard::Data> data, int operations);
  int DoDragWithOptions(std::vector<Clipboard::Data> data,
                        int operations,
                        const DragOptions& options);
  void CancelDrag();
  bool IsDragging() const;
  void RegisterDraggedTypes(std::set<Clipboard::Data::Type> types);

  // Custom cursor when mouse hovers the view.
  void SetCursor(scoped_refptr<Cursor> cursor);

  // Display related styles.
  virtual void SetFont(scoped_refptr<Font> font);
  virtual void SetColor(Color color);
  void SetBackgroundColor(Color color);

  // Set layout related styles without doing layout.
  // While this is public API, it should only be used by language bindings.
  void SetStyleProperty(const std::string& name, const std::string& value);
  void SetStyleProperty(const std::string& name, float value);

  // Set styles and re-compute the layout.
  template<typename... Args>
  void SetStyle(const std::string& name, const std::string& value,
                Args... args) {
    SetStyleProperty(name, value);
    SetStyle(args...);
    Layout();
  }
  template<typename... Args>
  void SetStyle(const std::string& name, float value, Args... args) {
    SetStyleProperty(name, value);
    SetStyle(args...);
    Layout();
  }
  void SetStyle() {
  }

  // Return the string representation of yoga style.
  std::string GetComputedLayout() const;

  // Return the minimum size of view.
  virtual SizeF GetMinimumSize() const;

#if defined(OS_MAC)
  void SetWantsLayer(bool wants);
  bool WantsLayer() const;
#endif

  // Get parent.
  View* GetParent() const { return parent_; }

  // Get window.
  Window* GetWindow() const { return window_; }

  // Get the native View object.
  NativeView GetNative() const { return view_; }

  // Internal: Set parent view.
  void SetParent(View* parent);
  void BecomeContentView(Window* window);
  void BecomeContentView();

  // Internal: Whether this class inherits from Container.
  virtual bool IsContainer() const;

  // Internal: Notify that view's size has changed.
  virtual void OnSizeChanged();

  // Internal: Get the CSS node of the view.
  YGNodeRef node() const { return node_; }

  // Internal: Return the overriden cursor.
  Cursor* cursor() const { return cursor_.get(); }

  // Internal: Return the overriden font.
  Font* font() const { return font_.get(); }

  // Events.
  Signal<void(View*, DraggingInfo*)> on_drag_leave;
  Signal<void(View*)> on_size_changed;
  Signal<void(View*)> on_capture_lost;

  // Delegates.
  std::function<int(View*, DraggingInfo*, const PointF&)> handle_drag_enter;
  std::function<int(View*, DraggingInfo*, const PointF&)> handle_drag_update;
  std::function<bool(View*, DraggingInfo*, const PointF&)> handle_drop;

 protected:
  View();
  ~View() override;

  // Update the default style.
  void UpdateDefaultStyle();

#if !defined(OS_LINUX)
  // SignalDelegate:
  void OnConnect(int identifier) override;
#endif

  // Called by subclasses to take the ownership of |view|.
  void TakeOverView(NativeView view);

  void PlatformDestroy();
  void PlatformSetVisible(bool visible);
  void PlatformSetCursor(Cursor* cursor);
  void PlatformSetFont(Font* font);

 private:
  friend class base::RefCounted<View>;

#if defined(OS_LINUX)
  // Whether events have been installed.
  bool on_drop_installed_ = false;
#endif

  // Relationships.
  View* parent_ = nullptr;
  Window* window_ = nullptr;

  // The native implementation.
  NativeView view_;

  // The config of its yoga node.
  YGConfigRef yoga_config_;

  // The font used for the view.
  scoped_refptr<Font> font_;

  // Custom cursor.
  scoped_refptr<Cursor> cursor_;

  // The node recording CSS styles.
  YGNodeRef node_;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
