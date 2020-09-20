// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include <algorithm>
#include <utility>

#include "nativeui/gfx/image.h"

namespace nu {

// static
const char GifPlayer::kClassName[] = "GifPlayer";

void GifPlayer::SetImage(scoped_refptr<Image> image) {
  image_ = std::move(image);
  PlatformSetImage(image_.get());
  UpdateDefaultStyle();
}

Image* GifPlayer::GetImage() const {
  return image_.get();
}

void GifPlayer::SetAnimating(bool animates) {
  // Reset timer.
  StopAnimationTimer();
  // Do not animate static image.
  if (!CanAnimate()) {
    is_animating_ = false;
    return;
  }
  is_animating_ = animates;
  // Create a timer to play animation.
  if (is_animating_ && IsTreeVisible())
    ScheduleFrame();
}

bool GifPlayer::IsAnimating() const {
  return is_animating_;
}

void GifPlayer::SetScale(ImageScale scale) {
  scale_ = scale;
  SchedulePaint();
}

ImageScale GifPlayer::GetScale() const {
  return scale_;
}

bool GifPlayer::IsPlaying() const {
  return timer_ != 0;
}

void GifPlayer::StopAnimationTimer() {
  if (timer_ != 0) {
    MessageLoop::ClearTimeout(timer_);
    timer_ = 0;
  }
}

void GifPlayer::Paint(Painter* painter) {
  // Calulate image position.
  RectF bounds = GetBounds();
  SizeF size = image_->GetSize();
  RectF rect;
  switch (scale_) {
    case ImageScale::None:
      rect = RectF((bounds.width() - size.width()) / 2,
                   (bounds.height() - size.height()) / 2,
                   size.width(), size.height());
      break;
    case ImageScale::Fill:
      rect = RectF(bounds.size());
      break;
    case ImageScale::Down:
      if (bounds.width() > size.width() && bounds.height() > size.height()) {
        rect = RectF((bounds.width() - size.width()) / 2,
                     (bounds.height() - size.height()) / 2,
                     size.width(), size.height());
        break;
      } else {
        // Fallthrough to ImageScale::UpOrDown code.
      }
    case ImageScale::UpOrDown: {
      float ratio = std::min(bounds.width() / size.width(),
                             bounds.height() / size.height());
      size = ScaleSize(size, ratio);
      rect = RectF((bounds.width() - size.width()) / 2,
                   (bounds.height() - size.height()) / 2,
                   size.width(), size.height());
      break;
    }
  }

  // Paint.
  painter->DrawImage(image_.get(), rect);
}

const char* GifPlayer::GetClassName() const {
  return kClassName;
}

SizeF GifPlayer::GetMinimumSize() const {
  if (image_ && scale_ == ImageScale::None)
    return image_->GetSize();
  else
    return SizeF();
}

}  // namespace nu
