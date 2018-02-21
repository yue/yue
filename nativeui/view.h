// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

typedef struct YGNode *YGNodeRef;
typedef struct YGConfig *YGConfigRef;

namespace nu {

class Font;
class Window;
struct MouseEvent;
struct KeyEvent;

// The base class for all kinds of views.
class NATIVEUI_EXPORT View : public base::RefCounted<View> {
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

  // Internal: The real pixel bounds that depends on the scale factor.
  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds() const;

  // Update layout.
  virtual void Layout();

  // Mark the whole view as dirty.
  void SchedulePaint();

  // Show/Hide the view.
  void SetVisible(bool visible);
  bool IsVisible() const;

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

  // Display related styles.
  void SetFont(Font* font);
  void SetColor(Color color);
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

  // Internal: Print style layout to stdout.
  void PrintStyle() const;

  // Return the minimum size of view.
  virtual SizeF GetMinimumSize() const;

#if defined(OS_MACOSX)
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

  // Internal: Notify that view's size has changed.
  virtual void OnSizeChanged();

  // Internal: Get the CSS node of the view.
  YGNodeRef node() const { return node_; }

  // Events.
  Signal<bool(View*, const MouseEvent&)> on_mouse_down;
  Signal<bool(View*, const MouseEvent&)> on_mouse_up;
  Signal<void(View*, const MouseEvent&)> on_mouse_move;
  Signal<void(View*, const MouseEvent&)> on_mouse_enter;
  Signal<void(View*, const MouseEvent&)> on_mouse_leave;
  Signal<bool(View*, const KeyEvent&)> on_key_down;
  Signal<bool(View*, const KeyEvent&)> on_key_up;
  Signal<void(View*)> on_size_changed;
  Signal<void(View*)> on_capture_lost;

 protected:
  View();
  virtual ~View();

  // Update the default style.
  void UpdateDefaultStyle();

  // Called by subclasses to take the ownership of |view|.
  void TakeOverView(NativeView view);

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetVisible(bool visible);

 private:
  friend class base::RefCounted<View>;

  // Relationships.
  View* parent_ = nullptr;
  Window* window_ = nullptr;

  // The native implementation.
  NativeView view_;

  // The config of its yoga node.
  YGConfigRef yoga_config_;

  // The font used for the view.
  scoped_refptr<Font> font_;

  // The node recording CSS styles.
  YGNodeRef node_;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
