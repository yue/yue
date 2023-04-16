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
    AppearanceBased = 0,
    Light = 1,
    Dark = 2,
    Titlebar = 3,
    Selection = 4,
    Menu = 5,
    Popover = 6,
    Sidebar = 7,
    HeaderView = 10,
    Sheet = 11,
    WindowBackground = 12,
    HUDWindow = 13,
    FullscreenUI = 15,
    Tooltip = 17,
    ContentBackground = 18,
    UnderWindowBackground = 21,
    UnderPageBackground = 22,
  };

  // Values here should match NSVisualEffectBlendingMode.
  enum class BlendingMode {
    BehindWindow = 0,
    WithinWindow = 1,
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
