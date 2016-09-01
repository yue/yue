// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_FILL_LAYOUT_H_
#define NATIVEUI_LAYOUT_FILL_LAYOUT_H_

#include "nativeui/layout/layout_manager.h"

namespace nu {

NATIVEUI_EXPORT class FillLayout : public LayoutManager {
 public:
  FillLayout();

  // LayoutManager:
  void Layout(Container* host) override;
  Size GetPreferredSize(Container* host) override;

 protected:
  ~FillLayout() override;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_FILL_LAYOUT_H_
