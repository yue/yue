// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CONTAINER_H_
#define NATIVEUI_CONTAINER_H_

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class Container : public View {
 public:
  Container();

 protected:
  void PlatformAddChildView(View* view) override;
  void PlatformRemoveChildView(View* view) override;

 protected:
  ~Container() override;
};

}  // namespace nu

#endif  // NATIVEUI_CONTAINER_H_
