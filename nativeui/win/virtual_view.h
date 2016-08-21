// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_VIRTUAL_VIEW_H_
#define NATIVEUI_WIN_VIRTUAL_VIEW_H_

#include "nativeui/win/base_view.h"

namespace nu {

// The view that draws directly into the window.
class VirtualView : public BaseView {
 public:
  VirtualView();

  void SetPixelBounds(const gfx::Rect& pixel_bounds) override;
  gfx::Rect GetPixelBounds() override;

  void SetParent(BaseView* parent) override;

 private:
  // The bounds relative to parent HWND.
  gfx::Rect bounds_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_VIRTUAL_VIEW_H_
