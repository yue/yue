// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLL_WIN_H_
#define NATIVEUI_WIN_SCROLL_WIN_H_

#include <memory>
#include <vector>

#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/scroll.h"
#include "nativeui/win/container_win.h"

namespace nu {

class Scrollbar;

// The implementation for the Scroll view.
class ScrollImpl : public ContainerImpl,
                   public ContainerImpl::Adapter {
 public:
  explicit ScrollImpl(Scroll* delegate);
  ~ScrollImpl() override;

  void SetOrigin(const Vector2d& origin);
  void SetContentSize(const Size& size);
  void SetScrollbarPolicy(Scroll::Policy h_policy, Scroll::Policy v_policy);

  Rect GetViewportRect() const;
  void OnScroll(int x, int y);

  // ContainerImpl::Adapter:
  void Layout() override;
  void ForEach(const std::function<bool(ViewImpl*)>& callback,
               bool reverse) override;
  bool HasChild(ViewImpl* child) override;

  // ViewImpl:
  void SizeAllocate(const Rect& size_allocation) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;
  bool OnMouseWheel(NativeEvent event) override;

  Scroll::Policy h_policy() const { return h_policy_; }
  Scroll::Policy v_policy() const { return v_policy_; }
  Size content_size() const { return content_size_; }
  Vector2d origin() const { return origin_; }
  Scroll* delegate() const { return delegate_; }

 private:
  void UpdateScrollbar();
  bool UpdateOrigin(Vector2d new_origin);
  Rect GetScrollbarRect(bool vertical) const;

  Size content_size_;
  Vector2d origin_;

  // The height of scrollbar.
  int scrollbar_height_;

  // The policy of showing scrollbar.
  Scroll::Policy h_policy_;
  Scroll::Policy v_policy_;

  std::unique_ptr<Scrollbar> h_scrollbar_;
  std::unique_ptr<Scrollbar> v_scrollbar_;

  Scroll* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLL_WIN_H_
