// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_PEN_H_
#define NATIVEUI_GFX_PEN_H_

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/color.h"
#include "nativeui/nativeui_export.h"

namespace nu {

NATIVEUI_EXPORT class Pen : public base::RefCounted<Pen> {
 public:
  Pen* Create(Color color, float width = 1.0);

 protected:
  Pen() = default;
  virtual ~Pen() = default;

 private:
  friend class base::RefCounted<Pen>;

  DISALLOW_COPY_AND_ASSIGN(Pen);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_PEN_H_
