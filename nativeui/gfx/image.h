// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_IMAGE_H_
#define NATIVEUI_GFX_IMAGE_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "nativeui/buffer.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/types.h"

#if defined(OS_WIN)
#include "base/win/scoped_gdi_object.h"
#endif

#if defined(OS_MAC)
#include <ImageIO/ImageIO.h>
#endif

#if defined(OS_LINUX)
typedef struct _GdkPixbufAnimationIter GdkPixbufAnimationIter;
#endif

namespace nu {

class NATIVEUI_EXPORT Image : public base::RefCounted<Image> {
 public:
  // Create an empty image.
  Image();

  // Take over an existing image.
  explicit Image(NativeImage take);

  // Create an image by reading from |path|.
  // The @2x suffix in basename will make the image have scale factor.
  explicit Image(const base::FilePath& path);

  // Create an image from memory.
  Image(const Buffer& buffer, float scale_factor);

  // Whether the image is empty.
  bool IsEmpty() const;

  // Get the size of image.
  SizeF GetSize() const;

  // Get the scale factor of image.
  float GetScaleFactor() const { return scale_factor_; }

  // Write the image to file.
  // Note: Do not make it a public API for now, we need to figure out a
  // universal type conversion API with options first.
  // Note: Should we add API for saving animations?
  bool WriteToFile(const std::string& format, const base::FilePath& target);

  // Return the native instance of image object.
  NativeImage GetNative() const { return image_; }

#if defined(OS_WIN)
  base::win::ScopedHICON GetHICON(const SizeF& size) const;
#endif

#if defined(OS_MAC)
  // Internal: Return the image representaion that has animations.
  NSBitmapImageRep* GetAnimationRep() const;

  // Internal: Get the duration of animations.
  float GetAnimationDuration(int index) const;
#endif

#if defined(OS_LINUX)
  // Internal: Advance the frame iter.
  void AdvanceFrame();

  // Internal: Return current animation frame.
  GdkPixbufAnimationIter* iter() const { return iter_; }
#endif

 protected:
  virtual ~Image();

 private:
  friend class base::RefCounted<Image>;

  static float GetScaleFactorFromFilePath(const base::FilePath& path);

  float scale_factor_ = 1.f;
  NativeImage image_;

#if defined(OS_LINUX)
  // GTK does not have concept of empty image.
  bool is_empty_ = false;
  // The animation frame.
  GdkPixbufAnimationIter* iter_ = nullptr;
#elif defined(OS_MAC)
  // The frame durations.
  std::vector<float> durations_;
#endif
};

}  // namespace nu

#endif  // NATIVEUI_GFX_IMAGE_H_
