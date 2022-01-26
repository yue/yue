// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CONTAINER_H_
#define NATIVEUI_CONTAINER_H_

#include <vector>

#include "nativeui/view.h"

namespace nu {

class Painter;

class NATIVEUI_EXPORT Container : public View {
 public:
  Container();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;
  void Layout() override;
  bool IsContainer() const override;
  void OnSizeChanged() override;

  // Gets preferred size of view.
  SizeF GetPreferredSize() const;

  // Returns the preferred width/height for the specified height/width.
  float GetPreferredHeightForWidth(float width) const;
  float GetPreferredWidthForHeight(float height) const;

  // Add/Remove children.
  void AddChildView(scoped_refptr<View> view);
  void AddChildViewAt(scoped_refptr<View> view, int index);
  void RemoveChildView(View* view);

  // Get children.
  int ChildCount() const { return static_cast<int>(children_.size()); }
  View* ChildAt(int index) const {
    if (index < 0 || index >= ChildCount())
      return nullptr;
    return children_[index].get();
  }

  // Internal: Used by certain implementations to refresh layout.
  virtual void UpdateChildBounds();

  // Events.
  Signal<void(Container*, Painter*, RectF)> on_draw;

 protected:
  ~Container() override;

  // Empty constructor used by subclasses.
  explicit Container(const char* an_empty_constructor);

  void PlatformInit();
  void PlatformDestroy();
  void PlatformAddChildView(View* view);
  void PlatformRemoveChildView(View* view);

 private:
  // Relationships.
  std::vector<scoped_refptr<View>> children_;

  // Whether the container should update children's layout.
  bool dirty_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_CONTAINER_H_
