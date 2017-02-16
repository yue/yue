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

namespace nu {

// The base class for all kinds of views.
class NATIVEUI_EXPORT View : public base::RefCounted<View> {
 public:
  NativeView view() const { return view_; }

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

  // Set CSS style for the node.
  void SetStyle(const std::string& name, const std::string& value);

  // Print style layout to stdout.
  void PrintStyle() const;

  // Get parent.
  View* parent() const { return parent_; }

  // Set parent, can only be used internally for now.
  void set_parent(View* parent) { parent_ = parent; }

  // Get the CSS node of the view, can only be used internally.
  YGNodeRef node() const { return node_; }

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

 private:
  friend class base::RefCounted<View>;

  // Relationships.
  View* parent_ = nullptr;

  // The native implementation.
  NativeView view_;

  // The node recording CSS styles.
  YGNodeRef node_;

  // Saved state of node's style.
  int node_position_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
