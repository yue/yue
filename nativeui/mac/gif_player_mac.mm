// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include "nativeui/gfx/image.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NUGifPlayer : NSImageView<NUView> {
 @private
  nu::NUPrivate private_;
}
@end

@implementation NUGifPlayer

- (void)setHidden:(BOOL)hidden {
  [super setHidden:hidden];
  // Start/stop animation when view visibility is changed.
  if (static_cast<nu::GifPlayer*>([self shell])->IsAnimating())
    [self setAnimates:!hidden];
}

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
}

- (void)setNUColor:(nu::Color)color {
}

- (void)setNUBackgroundColor:(nu::Color)color {
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

@end

namespace nu {

GifPlayer::GifPlayer() {
  TakeOverView([[NUGifPlayer alloc] init]);
  SetWantsLayer(true);
}

GifPlayer::~GifPlayer() {
}

void GifPlayer::PlatformSetImage(Image* image) {
  auto* gif = static_cast<NUGifPlayer*>(GetNative());
  gif.image = image ? image->GetNative() : nullptr;
  // Update property.
  is_animating_ = gif.animates;
  // Stop animation if view is hidden.
  if (gif.hidden)
    gif.animates = NO;
}

void GifPlayer::SetAnimating(bool animates) {
  auto* gif = static_cast<NUGifPlayer*>(GetNative());
  // We don't know if an image can animate, just set and read.
  gif.animates = animates;
  is_animating_ = gif.animates;
  // Stop animation if view is hidden.
  if (gif.hidden)
    gif.animates = NO;
}

}  // namespace nu
