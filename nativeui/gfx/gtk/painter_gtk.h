// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PAINTER_GTK_H_
#define NATIVEUI_GFX_GTK_PAINTER_GTK_H_

#include <stack>
#include <string>

#include "nativeui/gfx/painter.h"

namespace nu {

class PainterGtk : public Painter {
 public:
  // Create painter for |context|.
  PainterGtk(cairo_t* context, SizeF size);
  // Create graphics context for |surface| and use it for drawing.
  PainterGtk(cairo_surface_t* surface, SizeF size, float scale_factor);
  // PainterGtk should be created on stack for best performance.
  ~PainterGtk() override;

  // Painter:
  void Save() override;
  void Restore() override;
  void BeginPath() override;
  void ClosePath() override;
  void MoveTo(const PointF& point) override;
  void LineTo(const PointF& point) override;
  void BezierCurveTo(const PointF& cp1,
                     const PointF& cp2,
                     const PointF& ep) override;
  void Arc(const PointF& point, float radius, float sa, float ea) override;
  void Rect(const RectF& rect) override;
  void Clip() override;
  void ClipRect(const RectF& rect) override;
  void Translate(const Vector2dF& offset) override;
  void Rotate(float angle) override;
  void Scale(const Vector2dF& scale) override;
  void SetColor(Color color) override;
  void SetStrokeColor(Color color) override;
  void SetFillColor(Color color) override;
  void SetLineWidth(float width) override;
  void Stroke() override;
  void Fill() override;
  void Clear() override;
  void StrokeRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawPath() override;
  void DrawImage(const Image* image, const RectF& rect) override;
  void DrawImageFromRect(const Image* image, const RectF& src,
                         const RectF& dest) override;
  void DrawCanvas(Canvas* canvas, const RectF& rect) override;
  void DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                          const RectF& dest) override;
  void DrawAttributedText(scoped_refptr<AttributedText> text,
                          const RectF& rect) override;

 private:
  // Common initailization used by constructors.
  void Initialize();

  // Set source color from stroke or fill color.
  void SetSourceColor(bool stroke);

  // Cairo does not distinguish between stroke color and fill color, we have to
  // implement our own.
  struct PainterState {
    Color stroke_color;
    Color fill_color;
  };
  std::stack<PainterState> states_;

  // The nativeui drawing context.
  cairo_t* context_;

  SizeF size_;

  // Whether the context should be destroyed on exit.
  bool is_context_managed_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PAINTER_GTK_H_
