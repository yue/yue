// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include "nativeui/gfx/image.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

// Note that we don't use NSImageView because it sets the minimal frame duration
// to 100ms, which is too slow for progress indicators.
@interface NUGifPlayer : NSView<NUViewMethods> {
 @private
  nu::NUViewPrivate private_;
  nu::Color background_color_;
}
@end

@implementation NUGifPlayer

- (nu::NUViewPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
  background_color_ = color;
  [self setNeedsDisplay:YES];
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

- (BOOL)isFlipped {
  return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
  if (![self shell])
    return;

  nu::PainterMac painter(self);
  painter.SetColor(background_color_);
  painter.FillRect(nu::RectF(dirtyRect));

  auto* gif = static_cast<nu::GifPlayer*>([self shell]);
  gif->Paint(&painter);
}

- (void)viewDidHide {
  [super viewDidHide];
  auto* gif = static_cast<nu::GifPlayer*>([self shell]);
  if (gif && gif->IsAnimating())
    gif->StopAnimationTimer();
}

- (void)viewDidUnhide {
  [super viewDidUnhide];
  auto* gif = static_cast<nu::GifPlayer*>([self shell]);
  if (gif && gif->IsAnimating() && !gif->IsPlaying())
    gif->ScheduleFrame();
}

@end

namespace nu {

GifPlayer::GifPlayer() {
  TakeOverView([[NUGifPlayer alloc] init]);
  SetWantsLayer(true);
}

GifPlayer::~GifPlayer() {
  if (timer_ > 0)
    StopAnimationTimer();
}

void GifPlayer::PlatformSetImage(Image* image) {
  SchedulePaint();
  // Reset animation data.
  frames_count_ = 0;
  frame_ = 0;
  animation_rep_ = image ? image->GetAnimationRep() : nullptr;
  if (animation_rep_) {
    NSNumber* frames = [animation_rep_ valueForProperty:NSImageFrameCount];
    frames_count_ = [frames intValue];
    frame_ = frames_count_ - 1;
  }
  // Start animation by default.
  SetAnimating(!!image);
}

bool GifPlayer::CanAnimate() const {
  return animation_rep_ != nullptr;
}

void GifPlayer::ScheduleFrame() {
  // Advance frame.
  frame_ = (frame_ + 1) % frames_count_;
  [animation_rep_ setProperty:NSImageCurrentFrame
                    withValue:[NSNumber numberWithInteger:frame_]];
  // Emit draw event.
  SchedulePaint();
  // Schedule next call.
  if (is_animating_) {
    timer_ = MessageLoop::SetTimeout(
        image_->GetAnimationDuration(frame_),
        std::bind(&GifPlayer::ScheduleFrame, this));
  }
}

}  // namespace nu
