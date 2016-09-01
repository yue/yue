// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

// The base class for all kinds of views.
NATIVEUI_EXPORT class View : public base::RefCounted<View> {
 public:
  NativeView view() const { return view_; }

  // Change/Get position and size.
  void SetBounds(const Rect& bounds);
  Rect GetBounds() const;

  // The real pixel bounds that depends on the scale factor.
  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds() const;

  // Gets the offset to the parent window.
  Point GetWindowOrigin() const;
  Point GetWindowPixelOrigin() const;

  // Set the preferred size of the view, returns whether current view should
  // do a layout.
  bool SetPreferredSize(const Size& size);

  // A child changed its preferred size, returns true if the child should do a
  // layout.
  virtual bool UpdatePreferredSize();

  // Get the size the view would like to be.
  Size preferred_size() const { return preferred_size_; }

  // Get parent.
  View* parent() const { return parent_; }

  // Set parent, can only be used internally for now.
  void set_parent(View* parent) { parent_ = parent; }

 protected:
  View();
  virtual ~View();

  void set_view(NativeView view) { view_ = view; }

 private:
  friend class base::RefCounted<View>;

  // Relationships.
  View* parent_ = nullptr;

  // The native implementation.
  NativeView view_;

  // The preferred size of widget.
  Size preferred_size_;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
