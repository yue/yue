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

  // Values here should match NSVisualEffectMaterial.
  enum class Material {
    AppearanceBased,
    Light,
    Dark,
    Titlebar,
  };

  // Values here should match NSVisualEffectBlendingMode.
  enum class BlendingMode {
    BehindWindow,
    WithinWindow,
  };

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

  void SetMaterial(Material material);
  Material GetMaterial() const;
  void SetBlendingMode(BlendingMode mode);
  BlendingMode GetBlendingMode() const;

 protected:
  ~Vibrant() override;

  void PlatformInit();
};

}  // namespace nu

#endif  // NATIVEUI_VIBRANT_H_
