// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

class Container;

// The base class for all kinds of views.
NATIVEUI_EXPORT class View : public base::RefCounted<View> {
 public:
  NativeView view() const { return view_; }

  // Change/Get position and size.
  void SetBounds(const gfx::Rect& bounds);
  gfx::Rect GetBounds() const;

  // The real pixel bounds that depends on the scale factor.
  void SetPixelBounds(const gfx::Rect& bounds);
  gfx::Rect GetPixelBounds() const;

  // Gets the offset to the parent window.
  gfx::Point GetWindowOrigin() const;
  gfx::Point GetWindowPixelOrigin() const;

  // Get parent.
  const Container* parent() const { return parent_; }
  Container* parent() { return parent_; }

  // Set parent, can only be used internally for now.
  void set_parent(Container* parent) { parent_ = parent; }

 protected:
  View();
  virtual ~View();

  void set_view(NativeView view) { view_ = view; }

 private:
  friend class base::RefCounted<View>;

  // Relationships.
  Container* parent_ = nullptr;

  // The native implementation.
  NativeView view_;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
