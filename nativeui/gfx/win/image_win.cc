// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

Image::Image(const base::string16& path)
    : image_(new Gdiplus::Image(path.c_str())) {
}

Image::~Image() {
  delete image_;
}

Size Image::GetSize() const {
  Gdiplus::Image* image = const_cast<Gdiplus::Image*>(image_);
  return Size(image->GetWidth(), image->GetHeight());
}

NativeImage Image::GetNative() const {
  return image_;
}

}  // namespace nu
