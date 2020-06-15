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
  nu::RectF bounds = GetBounds();
  nu::SizeF size = image_->GetSize();
  nu::RectF rect((bounds.width() - size.width()) / 2,
                 (bounds.height() - size.height()) / 2,
                 size.width(), size.height());

  // Paint.
  painter->DrawImage(image_.get(), rect);
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
