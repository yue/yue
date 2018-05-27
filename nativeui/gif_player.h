// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GIF_PLAYER_H_
#define NATIVEUI_GIF_PLAYER_H_

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

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

#if defined(OS_LINUX)
  // Internal: Return current animation frame.
  GdkPixbufAnimationIter* GetFrame();

  // Internal: Pause the animation.
  void StopAnimationTimer();

  // Internal: Schedule to draw next animation frame.
  static gboolean ScheduleFrame(GifPlayer* self);
#endif

 protected:
  ~GifPlayer() override;

 private:
  void PlatformSetImage(Image* image);

#if defined(OS_LINUX)
  GdkPixbufAnimationIter* iter_ = nullptr;
  guint timer_ = 0;
#endif

  bool is_animating_ = false;
  scoped_refptr<Image> image_;
};

}  // namespace nu

#endif  // NATIVEUI_GIF_PLAYER_H_
