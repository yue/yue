// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_PAINTER_H_
#define NATIVEUI_GFX_PAINTER_H_

#include <memory>
#include <string>

#include "base/memory/weak_ptr.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/text.h"
#include "nativeui/types.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace nu {

class AttributedText;
class Canvas;
class Image;

// The interface for painting on canvas or window.
class NATIVEUI_EXPORT Painter {
 public:
  virtual ~Painter();

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

  // Fill the whole context with transparent color.
  virtual void Clear() = 0;

  // Draw a single pixel |rect|.
  virtual void StrokeRect(const RectF& rect) = 0;

  // Fill |rect|.
  virtual void FillRect(const RectF& rect) = 0;

#if defined(OS_LINUX)
  // Stroke and fill a |path|.
  virtual void DrawPath() = 0;
#endif

  // Draw image.
  virtual void DrawImage(const Image* image, const RectF& rect) = 0;
  virtual void DrawImageFromRect(const Image* image, const RectF& src,
                                 const RectF& dest) = 0;

  // Copy content from a canvas.
  virtual void DrawCanvas(Canvas* canvas, const RectF& rect) = 0;
  virtual void DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                                  const RectF& dest) = 0;

  // Draw attributed |text|.
  virtual void DrawAttributedText(scoped_refptr<AttributedText> text,
                                  const RectF& rect) = 0;

  // Draw |text| with additional |attributes|.
  virtual void DrawText(const std::string& text, const RectF& rect,
                        const TextAttributes& attributes);

  base::WeakPtr<Painter> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  Painter();

 private:
  base::WeakPtrFactory<Painter> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_PAINTER_H_
