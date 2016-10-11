// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_PAINTER_H_
#define NATIVEUI_GFX_PAINTER_H_

#include <memory>

#include "base/memory/weak_ptr.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/types.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace nu {

// The interface for painting on canvas or window.
class Painter {
 public:
#if defined(OS_WIN)
  // Create a painter from HDC.
  static std::unique_ptr<Painter> CreateFromHDC(HDC dc, float scale_factor);
#elif defined(OS_MACOSX)
  // Create a painter from current context.
  static std::unique_ptr<Painter> CreateFromCurrent();
#endif

  virtual ~Painter();

  enum {
    // Specifies the alignment for text rendered with the DrawText method.
    TextAlignLeft   = 1 << 0,
    TextAlignCenter = 1 << 1,
    TextAlignRight  = 1 << 2,
  };

  // How the clip rect is combined with current one.
  enum class CombineMode {
    Replace,
    Intersect,
    Union,
    Exclude,
  };

  // Save/Restore current state.
  virtual void Save() = 0;
  virtual void Restore() = 0;

  // Applies |rect| to the current clip using the specified region |op|.
  virtual void ClipRect(const RectF& rect,
                        CombineMode mode = CombineMode::Replace) = 0;

  // The origin offset of the painting.
  virtual void Translate(const Vector2dF& offset) = 0;

  // Sets the color used for drawing or filling.
  virtual void SetColor(Color color) = 0;

  // Draws a single pixel |rect|.
  virtual void DrawRect(const RectF& rect) = 0;

  // Fills |rect|.
  virtual void FillRect(const RectF& rect) = 0;

  // Draws text with the specified color, fonts and location. The text is
  // aligned to the left, vertically centered, clipped to the region. If the
  // text is too big, it is truncated and '...' is added to the end.
  void DrawText(const String& text, Font* font, const RectF& rect);

  // Draws text with the specified font and location. The last argument
  // specifies flags for how the text should be rendered.
  virtual void DrawTextWithFlags(
      const String& text, Font* font, const RectF& rect, int flags) = 0;

  base::WeakPtr<Painter> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  Painter();

 private:
  base::WeakPtrFactory<Painter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Painter);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_PAINTER_H_
