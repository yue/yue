// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/geometry/vector2d.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/container_win.h"

namespace nu {

namespace {

// The offset of title to left of rect.
const int kTitleLeftMargin = 5;

class GroupImpl : public ContainerImpl,
                  public ContainerImpl::Adapter {
 public:
  explicit GroupImpl(Group* delegate)
      : ContainerImpl(ControlType::Group, delegate, this),
        delegate_(delegate) {}

  void SetTitle(const base::string16& title) {
    title_ = title;
    OnDPIChanged();  // update component size
  }

  base::string16 GetTitle() const {
    return title_;
  }

  Insets GetBorder() const {
    int padding = std::ceil(2 * scale_factor());
    return Insets(title_bounds_.height() + padding, padding * 2,
                  padding * 2, padding * 2);
  }

  // ContainerImpl:
  void Layout() override {
    Rect child_alloc(size_allocation());
    child_alloc.Inset(GetBorder());
    delegate_->GetContentView()->GetNative()->SizeAllocate(child_alloc);
  }

  void ForEach(const std::function<bool(ViewImpl*)>& callback) override {
    callback(delegate_->GetContentView()->GetNative());
  }

  bool HasChild(ViewImpl* child) override {
    return child == delegate_->GetContentView()->GetNative();
  }

  // ViewImpl:
  void Draw(PainterWin* painter, const Rect& dirty) override {
    // Draw title.
    if (dirty.Intersects(title_bounds_)) {
      TextAttributes attributes(font(), color(), TextAlign::Center,
                                TextAlign::Center);
      painter->DrawTextPixel(title_, title_bounds_, attributes);
    }

    // Bounds of the content view.
    Rect child_bounds(size_allocation().size());
    Insets border = GetBorder();
    child_bounds.Inset(border);

    // Draw border.
    painter->BeginPath();
    painter->MoveToPixel(PointF(title_bounds_.x(),
                                child_bounds.y() - border.top() / 2.0f));
    painter->LineToPixel(PointF(child_bounds.x() - border.left() / 2.0f,
                                child_bounds.y()  - border.top() / 2.0f));
    painter->LineToPixel(PointF(child_bounds.x()   - border.left() / 2.0f,
                                child_bounds.bottom() + border.bottom() / 2.f));
    painter->LineToPixel(PointF(child_bounds.right()  + border.right() / 2,
                                child_bounds.bottom() + border.bottom() / 2.f));
    painter->LineToPixel(PointF(child_bounds.right() + border.right() / 2.f,
                                child_bounds.y()   - border.top() / 2.f));
    painter->LineToPixel(PointF(title_bounds_.right(),
                                child_bounds.y()   - border.top() / 2.f));
    painter->SetColor(Color(255, 170, 170, 170));
    painter->Stroke();

    // Draw child.
    child_bounds.Intersects(dirty);
    if (child_bounds.IsEmpty())
      return;
    Vector2d child_offset(border.left(), border.top());
    painter->Save();
    painter->ClipRectPixel(child_bounds);
    painter->TranslatePixel(child_offset);
    delegate_->GetContentView()->GetNative()->Draw(
        painter, child_bounds - child_offset);
    painter->Restore();
  }

  void OnDPIChanged() override {
    base::win::ScopedGetDC dc(window() ? window()->hwnd() : NULL);
    // Update the rect of the title.
    title_bounds_ = Rect(Point(kTitleLeftMargin * scale_factor(), 0),
                         ToCeiledSize(MeasureText(dc, title_, font())));
  }

 private:
  Group* delegate_;
  Rect title_bounds_;
  base::string16 title_;
};

}  // namespace

void Group::PlatformInit() {
  TakeOverView(new GroupImpl(this));
}

void Group::PlatformSetContentView(Container* container) {
  container->GetNative()->SetParent(GetNative());
  static_cast<GroupImpl*>(GetNative())->Layout();
}

void Group::SetTitle(const std::string& title) {
  static_cast<GroupImpl*>(GetNative())->SetTitle(base::UTF8ToUTF16(title));
  GetNative()->Invalidate();
}

std::string Group::GetTitle() const {
  return base::UTF16ToUTF8(static_cast<GroupImpl*>(GetNative())->GetTitle());
}

SizeF Group::GetBorderSize() const {
  GroupImpl* group = static_cast<GroupImpl*>(GetNative());
  Rect bounds;
  bounds.Inset(-group->GetBorder());
  return ScaleSize(SizeF(bounds.size()), 1.0f / group->scale_factor());
}

}  // namespace nu
