// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_H_
#define NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_H_

#include <vector>

#include "nativeui/win/scroll_win.h"
#include "nativeui/win/scrollbar/scrollbar_button.h"
#include "nativeui/win/scrollbar/scrollbar_thumb.h"

namespace nu {

// Draws the native scroll bar.
class Scrollbar : public ContainerImpl,
                  public ContainerImpl::Adapter {
 public:
  Scrollbar(bool vertical, ScrollImpl* scroll);
  ~Scrollbar() override;

  void LineUp();
  void LineDown();
  void PageUp();
  void PageDown();

  int GetValue() const;
  void SetValue(int value);

  // ContainerImpl::Adapter:
  void Layout() override;
  void ForEach(const std::function<bool(ViewImpl*)>& callback,
               bool reverse) override;
  bool HasChild(ViewImpl* child) override;

  // ViewImpl:
  void OnMouseEnter(NativeEvent event) override;
  void OnMouseLeave(NativeEvent event) override;
  bool OnMouseClick(NativeEvent event) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

 private:
  void UpdateThumbPosition();
  void OnClick();

  int GetTrackSize() const;
  int GetBoxSize() const;
  int GetScrollAmout() const;
  int GetLineHeight() const;
  int GetPageHeight() const;

  NativeTheme::ScrollbarTrackExtraParams params_ = {0};

  int contents_size_ = 1;
  int viewport_size_ = 1;

  ScrollbarButton near_button_;
  ScrollbarButton far_button_;
  ScrollbarThumb thumb_;

  RepeatController repeater_;

  bool vertical_;
  ScrollImpl* scroll_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_H_
