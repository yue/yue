// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIBRANT_H_
#define NATIVEUI_VIBRANT_H_

#include "nativeui/container.h"

namespace nu {

class NATIVEUI_EXPORT Vibrant : public Container {
 public:
  Vibrant();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

 protected:
  ~Vibrant() override;

  void PlatformInit();
};

}  // namespace nu

#endif  // NATIVEUI_VIBRANT_H_
