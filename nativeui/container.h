// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CONTAINER_H_
#define NATIVEUI_CONTAINER_H_

#include <vector>

#include "nativeui/view.h"

namespace nu {

class Painter;

NATIVEUI_EXPORT class Container : public View {
 public:
  Container();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;
  void Layout() override;

  // Called when view's bounds changed, and probably needs layout.
  void BoundsChanged();

  // Gets preferred size of view.
  SizeF GetPreferredSize() const;

  // Returns the preferred width/height for the specified height/width.
  float GetPreferredHeightForWidth(float width) const;
  float GetPreferredWidthForHeight(float height) const;

  // Add/Remove children.
  void AddChildView(View* view);
  void AddChildViewAt(View* view, int index);
  void RemoveChildView(View* view);

  // Get children.
  int child_count() const { return static_cast<int>(children_.size()); }
  View* child_at(int index) const {
    if (index < 0 || index >= child_count())
      return nullptr;
    return children_[index].get();
  }

  // Events.
  Signal<void(Container*, Painter*, const RectF&)> on_draw;

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
