// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_IMAGE_GTK_H_
#define NATIVEUI_GFX_GTK_IMAGE_GTK_H_

#include <string>

#include "nativeui/gfx/image.h"

namespace nu {

NATIVEUI_EXPORT class ImageGtk : public Image {
 public:
  ImageGtk(const std::string& path);

  // Image:
  Size GetSize() const override;
  NativeImage GetNative() const override;

 private:
  ~ImageGtk() override;

  NativeImage image_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_IMAGE_GTK_H_
