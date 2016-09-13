// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_CSS_LAYOUT_H_
#define NATIVEUI_LAYOUT_CSS_LAYOUT_H_

#include "nativeui/layout/layout_manager.h"

namespace nu {

class CSSLayout : public LayoutManager {
 public:
  CSSLayout();

  // LayoutManager:
  void Layout(Container* host) const override;
  Size GetPixelPreferredSize(Container* host) const override;

 protected:
  ~CSSLayout() override;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_CSS_LAYOUT_H_
