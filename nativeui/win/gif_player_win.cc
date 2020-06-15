// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include "nativeui/gfx/image.h"
#include "nativeui/win/view_win.h"

namespace nu {

namespace {

class GifPlayerImpl : public ViewImpl {
 public:
  explicit GifPlayerImpl(GifPlayer* delegate)
      : ViewImpl(ControlType::View, delegate) {}

 protected:
  // ViewImpl:
  void Draw(PainterWin* painter, const Rect& dirty) override {
    ViewImpl::Draw(painter, dirty);

    auto* gif = static_cast<GifPlayer*>(delegate());
    gif->Paint(painter);
  }

  void VisibilityChanged() override {
    ViewImpl::VisibilityChanged();
    // Start/stop animation when view visibility is changed.
    auto* gif = static_cast<GifPlayer*>(delegate());
    if (gif->IsAnimating() && (is_tree_visible() != gif->IsPlaying())) {
      if (is_tree_visible())
        gif->ScheduleFrame();
      else
        gif->StopAnimationTimer();
    }
  }
};

}  // namespace

GifPlayer::GifPlayer() {
  TakeOverView(new GifPlayerImpl(this));
}

GifPlayer::~GifPlayer() {
  if (timer_ != 0)
    StopAnimationTimer();
}

void GifPlayer::PlatformSetImage(Image* image) {
  // Reset frame information after chaning image.
  frames_count_ = 0;
  frame_ = 0;
  frame_delays_.reset();
  if (image) {
    // Frame dimensions:
    // frames[animation_frame_index][how_many_animation]
    UINT dimensions_count = image->GetNative()->GetFrameDimensionsCount();
    if (dimensions_count > 0) {
      // Get the identifiers for the frame dimensions.
      std::vector<GUID> ids(dimensions_count);
      image->GetNative()->GetFrameDimensionsList(ids.data(), dimensions_count);
      // For GIF, only #0 is meaningful.
      frames_count_ = image->GetNative()->GetFrameCount(&ids[0]);
      frame_ = frames_count_ - 1;
      // Get frame delays.
      if (frames_count_ > 1) {
        UINT size = image->GetNative()->GetPropertyItemSize(
            PropertyTagFrameDelay);
        frame_delays_.reset(new BYTE[size]);
        image->GetNative()->GetPropertyItem(
            PropertyTagFrameDelay, size,
            reinterpret_cast<Gdiplus::PropertyItem*>(frame_delays_.get()));
      }
    }
  }
  // Start animation by default.
  SetAnimating(!!image);
  GetNative()->Invalidate();
}

bool GifPlayer::CanAnimate() const {
  return frames_count_ > 1;
}

void GifPlayer::ScheduleFrame() {
  // Advance frame.
  frame_ = (frame_ + 1) % frames_count_;
  image_->GetNative()->SelectActiveFrame(&Gdiplus::FrameDimensionTime, frame_);
  // Emit draw event.
  SchedulePaint();
  // Schedule next call.
  if (is_animating_) {
    auto* item = reinterpret_cast<Gdiplus::PropertyItem*>(frame_delays_.get());
    auto* delays = static_cast<UINT*>(item->value);
    timer_ = MessageLoop::SetTimeout(
        delays[frame_] * 10, std::bind(&GifPlayer::ScheduleFrame, this));
  }
}

}  // namespace nu
