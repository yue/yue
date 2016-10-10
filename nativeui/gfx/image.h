// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_IMAGE_H_
#define NATIVEUI_GFX_IMAGE_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

NATIVEUI_EXPORT class Image : public base::RefCounted<Image> {
 public:
  Image(const String& file);

  NativeImage image() const { return image_; }

 private:
  friend class base::RefCounted<Image>;

  ~Image();

  NativeImage image_;

  DISALLOW_COPY_AND_ASSIGN(Image);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_IMAGE_H_
