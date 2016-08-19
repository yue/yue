// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"
#include "ui/gfx/geometry/rect.h"

namespace nu {

// The base class for all kinds of views.
NATIVEUI_EXPORT class View : public base::RefCounted<View> {
 public:
  NativeView view() const { return view_; }

  // Add/Remove children.
  void AddChildView(View* view);
  void AddChildViewAt(View* view, int index);
  void RemoveChildView(View* view);

  // Get children.
  int child_count() const { return static_cast<int>(children_.size()); }
  bool has_children() const { return !children_.empty(); }
  View* child_at(int index) const {
    if (index < 0 || index >= child_count())
      return nullptr;
    return children_[index].get();
  }

  // Get parent.
  const View* parent() const { return parent_; }
  View* parent() { return parent_; }

  // Change/Get position and size.
  void SetBounds(const gfx::Rect& bounds);
  gfx::Rect GetBounds();

 protected:
  View();
  virtual ~View();

  void set_view(NativeView view) { view_ = view; }

  // Platform-dependent implementations.
  virtual void PlatformAddChildView(View* view);
  virtual void PlatformRemoveChildView(View* view);

 private:
  friend class base::RefCounted<View>;

  // Relationships.
  View* parent_ = nullptr;
  std::vector<scoped_refptr<View>> children_;

  // The native implementation.
  NativeView view_;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
