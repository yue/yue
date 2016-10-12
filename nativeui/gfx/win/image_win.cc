// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/image_win.h"

namespace nu {

ImageWin::ImageWin(const base::string16& path) : image_(path.c_str()) {
}

ImageWin::~ImageWin() {
}

Size ImageWin::GetSize() const {
  ImageWin* self = const_cast<ImageWin*>(this);
  return Size(self->image_.GetWidth(), self->image_.GetHeight());
}

NativeImage ImageWin::GetNative() const {
  return const_cast<NativeImage>(&image_);
}

// static
Image* Image::CreateFromFile(const String& path) {
  return new ImageWin(path);
}

}  // namespace nu
