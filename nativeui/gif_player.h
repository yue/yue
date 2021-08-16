// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GIF_PLAYER_H_
#define NATIVEUI_GIF_PLAYER_H_

#include <memory>

#include "nativeui/gfx/painter.h"
#include "nativeui/message_loop.h"
#include "nativeui/standard_enums.h"
#include "nativeui/view.h"

namespace nu {

class Image;
class Painter;

class NATIVEUI_EXPORT GifPlayer : public View {
 public:
  GifPlayer();

  // View class name.
  static const char kClassName[];

  void SetImage(scoped_refptr<Image> image);
  Image* GetImage() const;
  void SetAnimating(bool animates);
  bool IsAnimating() const;
  void SetScale(ImageScale scale);
  ImageScale GetScale() const;

  // Internal: Is animation being played.
  bool IsPlaying() const;

  // Internal: Pause the animation.
  void StopAnimationTimer();

  // Internal: Draw the animation frame.
  void Paint(Painter* painter);

  // Internal: Whether the image can animate.
  bool CanAnimate() const;

  // Internal: Schedule to draw next animation frame.
  void ScheduleFrame();

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

 protected:
  ~GifPlayer() override;

 private:
  void PlatformSetImage(Image* image);

#if defined(OS_MAC)
  unsigned int frames_count_ = 0;
  unsigned int frame_ = 0;
  NSBitmapImageRep* animation_rep_ = nullptr;
#elif defined(OS_WIN)
  UINT frames_count_ = 0;
  UINT frame_ = 0;
  std::unique_ptr<BYTE[]> frame_delays_;
#endif

  MessageLoop::TimerId timer_ = 0;

  bool is_animating_ = false;
  ImageScale scale_ = ImageScale::None;
  scoped_refptr<Image> image_;
};

}  // namespace nu

#endif  // NATIVEUI_GIF_PLAYER_H_
