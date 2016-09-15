// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CONTAINER_H_
#define NATIVEUI_CONTAINER_H_

#include <vector>

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class Container : public View {
 public:
  Container();

  // View class name.
  static const char kClassName[];

  // View:
  void UpdatePreferredSize() override;
  const char* GetClassName() const override;

  // Called when the view needs layout aggresively.
  virtual void Layout();

  // Called when view's bounds changed, and probably needs layout.
  void BoundsChanged();

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

 protected:
  ~Container() override;

  void SetChildBoundsFromCSS();

  void PlatformInit();
  void PlatformDestroy();
  void PlatformAddChildView(View* view);
  void PlatformRemoveChildView(View* view);

 private:
  // Relationships.
  std::vector<scoped_refptr<View>> children_;
};

}  // namespace nu

#endif  // NATIVEUI_CONTAINER_H_
