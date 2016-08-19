// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CONTAINER_H_
#define NATIVEUI_CONTAINER_H_

#include "nativeui/view.h"

namespace nu {

class LayoutManager;

NATIVEUI_EXPORT class Container : public View {
 public:
  Container();

  void SetLayoutManager(LayoutManager* layout_manager);
  LayoutManager* GetLayoutManager() const;

  // View:
  void Layout() override;

 protected:
  ~Container() override;

  void PlatformInit();

 private:
  scoped_refptr<LayoutManager> layout_manager_;
};

}  // namespace nu

#endif  // NATIVEUI_CONTAINER_H_
