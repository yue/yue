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
class NATIVEUI_EXPORT Painter {
 public:
  virtual ~Painter();

  enum {
    // Specifies the alignment for text rendered with the DrawText method.
    TextAlignLeft   = 1 << 0,
    TextAlignCenter = 1 << 1,
    TextAlignRight  = 1 << 2,
  };

  // Save/Restore current state.
  virtual void Save() = 0;
  virtual void Restore() = 0;

  // Path operations.
  virtual void BeginPath() = 0;
  virtual void ClosePath() = 0;
  virtual void MoveTo(const PointF& p) = 0;
  virtual void LineTo(const PointF& p) = 0;
  virtual void BezierCurveTo(const PointF& cp1,
                             const PointF& cp2,
                             const PointF& ep) = 0;
  virtual void ArcTo(const PointF& cp1, const PointF& cp2, float radius) = 0;
  virtual void Arc(const PointF& point, float radius, float sa, float ea) = 0;
  virtual void Rect(const RectF& rect) = 0;

  // Add current path to clip area by intersection.
  virtual void Clip() = 0;

  // Apply |rect| to the current clip using the specified region |op|.
  virtual void ClipRect(const RectF& rect) = 0;

  // Transform operations.
  virtual void Translate(const Vector2dF& offset) = 0;
  virtual void Rotate(float angle) = 0;
  virtual void Scale(const Vector2dF& scale) = 0;

  // Set the color used for drawing or filling.
  virtual void SetColor(Color color) = 0;
  virtual void SetStrokeColor(Color color) = 0;
  virtual void SetFillColor(Color color) = 0;

  // Set the width used for drawing lines.
  virtual void SetLineWidth(float width) = 0;

  // Stroke current path.
  virtual void Stroke() = 0;

  // Draw a solid shape by filling current path's content area.
  virtual void Fill() = 0;

  // Draw a single pixel |rect|.
  virtual void StrokeRect(const RectF& rect) = 0;

  // Fill |rect|.
  virtual void FillRect(const RectF& rect) = 0;

  // Return the space taken to paint the full string.
  virtual SizeF MeasureText(base::StringPiece text, Font* font) = 0;

  // Draw text with the specified color, fonts and location. The text is
  // aligned to the left, vertically centered, clipped to the region. If the
  // text is too big, it is truncated and '...' is added to the end.
  void DrawText(base::StringPiece text, Font* font, const RectF& rect);

  // Draw text with the specified font and location. The last argument
  // specifies flags for how the text should be rendered.
  void DrawTextWithFlags(
      base::StringPiece text, Font* font, const RectF& rect, int flags);

  // Draw colored text.
  void DrawColoredText(
      base::StringPiece text, Font* font, Color color, const RectF& rect);

  // Draw colored text with additional flags.
  virtual void DrawColoredTextWithFlags(
      base::StringPiece text, Font* font, Color color, const RectF& rect,
      int flags) = 0;

  base::WeakPtr<Painter> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  Painter();

 private:
  base::WeakPtrFactory<Painter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Painter);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_PAINTER_H_
