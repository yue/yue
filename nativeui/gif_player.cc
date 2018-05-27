// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include "nativeui/gfx/image.h"

namespace nu {

// static
const char GifPlayer::kClassName[] = "GifPlayer";

void GifPlayer::SetImage(Image* image) {
  image_ = image;
  PlatformSetImage(image);
  UpdateDefaultStyle();
}

Image* GifPlayer::GetImage() const {
  return image_.get();
}

const char* GifPlayer::GetClassName() const {
  return kClassName;
}

SizeF GifPlayer::GetMinimumSize() const {
  if (image_)
    return image_->GetSize();
  else
    return SizeF();
}

}  // namespace nu
