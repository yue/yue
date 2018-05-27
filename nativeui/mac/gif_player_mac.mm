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
  TakeOverView([[NSImageView alloc] init]);
}

GifPlayer::~GifPlayer() {
}

void GifPlayer::PlatformSetImage(Image* image) {
  NSImage* nsimage = image ? image->GetNative() : nullptr;
  static_cast<NUGifPlayer*>(GetNative()).image = nsimage;
}

void GifPlayer::SetAnimating(bool animates) {
  static_cast<NUGifPlayer*>(GetNative()).animates = animates;
}

bool GifPlayer::IsAnimating() const {
  return static_cast<NUGifPlayer*>(GetNative()).animates;
}

}  // namespace nu
