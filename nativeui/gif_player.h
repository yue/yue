// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GIF_PLAYER_H_
#define NATIVEUI_GIF_PLAYER_H_

#include <memory>

#include "nativeui/message_loop.h"
#include "nativeui/view.h"

#if defined(OS_LINUX)
typedef struct _GdkPixbufAnimationIter GdkPixbufAnimationIter;
#endif

namespace nu {

class Image;

class NATIVEUI_EXPORT GifPlayer : public View {
 public:
  GifPlayer();

  // View class name.
  static const char kClassName[];

  void SetImage(Image* image);
  Image* GetImage() const;
  void SetAnimating(bool animates);
  bool IsAnimating() const;

  // Internal: Is animation being played.
  bool IsPlaying() const;

  // Internal: Pause the animation.
  void StopAnimationTimer();

  // Internal: Whether the image can animate.
  bool CanAnimate() const;

  // Internal: Schedule to draw next animation frame.
  void ScheduleFrame();

#if defined(OS_LINUX)
  // Internal: Return current animation frame.
  GdkPixbufAnimationIter* GetFrame();
#endif

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

 protected:
  ~GifPlayer() override;

 private:
  void PlatformSetImage(Image* image);

#if defined(OS_LINUX)
  GdkPixbufAnimationIter* iter_ = nullptr;
#elif defined(OS_MACOSX)
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
  scoped_refptr<Image> image_;
};

}  // namespace nu

#endif  // NATIVEUI_GIF_PLAYER_H_
