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

struct MouseEvent;

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

  // Change/Get position and size.
  void SetBounds(const RectF& bounds);
  RectF GetBounds() const;

  // The real pixel bounds that depends on the scale factor.
  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds() const;

  // Update layout.
  virtual void Layout();

  // Show/Hide the view.
  void SetVisible(bool visible);
  bool IsVisible() const;

  // Set backgroundcolor.
  void SetBackgroundColor(Color color);
  Color GetBackgroundColor() const;

  // Set CSS style for the node.
  void SetStyle(const std::string& name, const std::string& value);

  // Print style layout to stdout.
  void PrintStyle() const;

  // Get parent.
  View* GetParent() const { return parent_; }

  // Get the native View object.
  NativeView GetNative() const { return view_; }

  // Internal: Set parent view.
  void SetParent(View* parent);

  // Internal: Get the CSS node of the view.
  YGNodeRef node() const { return node_; }

  // Events.
  Signal<bool(View*, const MouseEvent&)> on_mouse_down;
  Signal<bool(View*, const MouseEvent&)> on_mouse_up;

 protected:
  View();
  virtual ~View();

  // Set the default style of view.
  void SetDefaultStyle(const SizeF& minimum);

  // Called by subclasses to take the ownership of |view|.
  void TakeOverView(NativeView view);

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetVisible(bool visible);
  void PlatformSetBackgroundColor(Color color);

 private:
  friend class base::RefCounted<View>;

  // Background color of View.
  Color background_color_;

  // Relationships.
  View* parent_ = nullptr;

  // The native implementation.
  NativeView view_;

  // The config of its yoga node.
  YGConfigRef yoga_config_;

  // The node recording CSS styles.
  YGNodeRef node_;

  // Saved state of node's style.
  int node_position_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
