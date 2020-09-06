// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_THUMB_H_
#define NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_THUMB_H_

#include "nativeui/gfx/win/native_theme.h"
#include "nativeui/win/clickable.h"

namespace nu {

class Scrollbar;

class ScrollbarThumb : public Clickable {
 public:
  ScrollbarThumb(bool vertical, Scrollbar* scrollbar);
  ~ScrollbarThumb() override;

  int GetSize() const;

  // ViewImpl:
  void OnMouseMove(NativeEvent event) override;
  bool OnMouseClick(NativeEvent event) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

  NativeTheme::ScrollbarThumbExtraParams* params() { return &params_; }

 private:
  NativeTheme::ScrollbarThumbExtraParams params_ = {0};

  Point pressed_point_;
  int last_value_ = 0;

  bool vertical_;
  Scrollbar* scrollbar_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_THUMB_H_
