// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_DRAWING_EFFECT_H_
#define NATIVEUI_GFX_WIN_DRAWING_EFFECT_H_

#include <dwrite.h>

#include "base/macros.h"
#include "nativeui/gfx/color.h"

namespace nu {

class DrawingEffect : public IUnknown {
 public:
  DrawingEffect();
  ~DrawingEffect();

  Color fg_color;

  // IUnknown:
  HRESULT __stdcall QueryInterface(const IID& riid,
                                   void** ppvObject) override;
  ULONG __stdcall AddRef() override;
  ULONG __stdcall Release() override;

 private:
  LONG ref_ = 0;

  DISALLOW_COPY_AND_ASSIGN(DrawingEffect);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_DRAWING_EFFECT_H_
