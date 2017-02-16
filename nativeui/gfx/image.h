// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_IMAGE_H_
#define NATIVEUI_GFX_IMAGE_H_

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/geometry/size.h"
#include "nativeui/types.h"

namespace nu {

class NATIVEUI_EXPORT Image : public base::RefCounted<Image> {
 public:
  // Creates an image by reading from |path|.
  static Image* CreateFromFile(const String& path);

  // Gets the size of image.
  virtual Size GetSize() const = 0;

  // Returns the native instance of image object.
  virtual NativeImage GetNative() const = 0;

 protected:
  virtual ~Image() = default;

 private:
  friend class base::RefCounted<Image>;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_IMAGE_H_
