// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_IMAGE_H_
#define NATIVEUI_GFX_IMAGE_H_

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/types.h"

namespace nu {

class NATIVEUI_EXPORT Image : public base::RefCounted<Image> {
 public:
  // Create an image by reading from |path|.
  // The @2x suffix in basename will make the image have scale factor.
  explicit Image(const base::FilePath& path);

  // Get the size of image.
  SizeF GetSize() const;

  // Get the scale factor of image.
  float GetScaleFactor() const { return scale_factor_; }

  // Return the native instance of image object.
  NativeImage GetNative() const;

 protected:
  virtual ~Image();

 private:
  friend class base::RefCounted<Image>;

  static float GetScaleFactorFromFilePath(const base::FilePath& path);

  float scale_factor_;
  NativeImage image_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_IMAGE_H_
