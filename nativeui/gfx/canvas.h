// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_CANVAS_H_
#define NATIVEUI_GFX_CANVAS_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class Painter;

class NATIVEUI_EXPORT Canvas : public base::RefCounted<Canvas> {
 public:
  // Create a canvas with the default scale factor.
  // This is strongly discouraged for using, since it does not work well with
  // multi-monitor setup, but honestly I don't know whether there is a good
  // way to handle per-monitor DPI for canvas.
  explicit Canvas(const SizeF& size);
  // Create a canvas with |scale_factor|.
  Canvas(const SizeF& size, float scale_factor);

  // Return the independent scale factor of canvas.
  float GetScaleFactor() const { return scale_factor_; }

  // Return the Painter that can be used to draw on canvas.
  Painter* GetPainter() { return painter_.get(); }

  // Return the size of canvas.
  SizeF GetSize() const { return size_; }

  // Internal: Return the native bitmap object.
  NativeBitmap GetBitmap() const { return bitmap_; }

 protected:
  virtual ~Canvas();

 private:
  friend class base::RefCounted<Canvas>;

  // Platform implementations.
  static NativeBitmap PlatformCreateBitmap(const SizeF& size,
                                           float scale_factor);
  static void PlatformDestroyBitmap(NativeBitmap bitmap);
  static Painter* PlatformCreatePainter(NativeBitmap bitmap,
                                        const SizeF& size,
                                        float scale_factor);

  float scale_factor_;
  SizeF size_;

  NativeBitmap bitmap_;
  std::unique_ptr<Painter> painter_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_CANVAS_H_
