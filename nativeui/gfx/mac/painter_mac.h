// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_MAC_PAINTER_MAC_H_
#define NATIVEUI_GFX_MAC_PAINTER_MAC_H_

#include <string>

#include "nativeui/gfx/painter.h"

typedef struct CGContext* CGContextRef;

namespace nu {

class PainterMac : public Painter {
 public:
  // Create painter for the current context.
  PainterMac();
  // Create graphics context for |bitmap| and use it for drawing.
  PainterMac(CGContextRef bitmap, float scale_factor);
  // PainterMac should be created on stack for best performance.
  ~PainterMac() override;

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
  void StrokeRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawImage(Image* image, const RectF& rect) override;
  void DrawImageFromRect(Image* image, const RectF& src,
                         const RectF& dest) override;
  void DrawCanvas(Canvas* canvas, const RectF& rect) override;
  void DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                          const RectF& dest) override;
  void DrawAttributedText(AttributedText* text, const RectF& rect) override;

 private:
  // APIs of Core Graphics operate on current context, while we don't set
  // current context for memory bitmap. So in order to support Canvas we have
  // to save the context object and do manual context switching.
  NSGraphicsContext* target_context_;

  CGContextRef context_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_MAC_PAINTER_MAC_H_
