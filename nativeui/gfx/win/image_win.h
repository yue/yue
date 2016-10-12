// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_IMAGE_WIN_H_
#define NATIVEUI_GFX_WIN_IMAGE_WIN_H_

#include "nativeui/gfx/image.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

NATIVEUI_EXPORT class ImageWin : public Image {
 public:
  ImageWin(const base::string16& path);

  // Image:
  Size GetSize() const override;
  NativeImage GetNative() const override;

 private:
  ~ImageWin() override;

  Gdiplus::Image image_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_IMAGE_WIN_H_
