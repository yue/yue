// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_CONTAINER_WIN_H_
#define NATIVEUI_WIN_CONTAINER_WIN_H_

#include <functional>
#include <vector>

#include "nativeui/container.h"
#include "nativeui/win/view_win.h"

namespace nu {

// The common base for views that have children.
class ContainerImpl : public ViewImpl {
 public:
  class Adapter {
   public:
    virtual ~Adapter() = default;

    virtual void Layout() = 0;
    virtual void ForEach(const std::function<bool(ViewImpl*)>& callback,
                         bool reverse = false) = 0;
    virtual bool HasChild(ViewImpl* child) = 0;
    virtual void OnDraw(PainterWin* painter, const Rect& dirty) {}
  };

  ContainerImpl(View* view, Adapter* delegate,
                ControlType type = ControlType::Container);

  // Baseview:
  void SizeAllocate(const Rect& size_allocation) override;
  UINT HitTest(const Point& point) const override;
  void SetParent(ViewImpl* parent) override;
  void BecomeContentView(WindowImpl* parent) override;
  void SetVisible(bool visible) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;
  void OnMouseMove(NativeEvent event) override;
  void OnMouseLeave(NativeEvent event) override;
  bool OnMouseWheel(bool vertical, UINT flags, int delta,
                    const Point& point) override;
  bool OnMouseClick(NativeEvent event) override;

  Adapter* adapter() const { return adapter_; }

 protected:
  void DrawChild(ViewImpl* child, PainterWin* painter, const Rect& dirty);

 private:
  void RefreshParentTree();
  ViewImpl* FindChildFromPoint(const Point& point) const;

  Adapter* adapter_;

  // The View in which mouse hovers.
  ViewImpl* hover_view_ = nullptr;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_CONTAINER_WIN_H_
