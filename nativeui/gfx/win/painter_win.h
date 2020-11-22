// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PAINTER_WIN_H_
#define NATIVEUI_GFX_WIN_PAINTER_WIN_H_

#include <stack>
#include <string>
#include <utility>

#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/gfx/win/native_theme.h"

namespace nu {

class PainterWin : public Painter {
 public:
  // Paint on the HDC.
  PainterWin(HDC hdc, Size size, float scale_factor);
  // PainterWin should be created on stack for best performance.
  ~PainterWin() override;

  // Draw a control.
  void DrawNativeTheme(NativeTheme::Part part,
                       ControlState state,
                       const nu::Rect& rect,
                       const NativeTheme::ExtraParams& extra);

  // Draw the focus rect.
  void DrawFocusRect(const nu::Rect& rect);

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
  void DrawImage(const Image* image, const RectF& rect) override;
  void DrawImageFromRect(const Image* image, const RectF& src,
                         const RectF& dest) override;
  void DrawCanvas(Canvas* canvas, const RectF& rect) override;
  void DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                          const RectF& dest) override;
  void DrawAttributedText(scoped_refptr<AttributedText> text,
                          const RectF& rect) override;

  // Internal: The pixel versions.
  void MoveToPixel(const PointF& point);
  void LineToPixel(const PointF& point);
  void BezierCurveToPixel(const PointF& cp1,
                          const PointF& cp2,
                          const PointF& ep);
  void ArcPixel(const PointF& point, float r, float sa, float ea,
                bool anticlockwise);
  void RectPixel(const RectF& rect);
  void ClipRectPixel(const nu::Rect& rect);
  void TranslatePixel(const Vector2d& offset);
  void StrokeRectPixel(const nu::Rect& rect);
  void FillRectPixel(const nu::Rect& rect);

  // Internal: Save with the information of relevant size.
  void SaveWithSize(SizeF size);
  void SaveWithSize(Size size);

 private:
  // Used for common initialization.
  void Initialize(Size size, float scale_factor);

  // Get current point.
  bool GetCurrentPoint(Gdiplus::PointF* point);

  // Receive the HDC that can be painted on.
  HDC GetHDC();
  void ReleaseHDC(HDC dc);

  // The saved state.
  struct PainterState {
    PainterState(Size size,
                 float line_width,
                 Color stroke_color,
                 Color fill_color)
        : size(std::move(size)),
          line_width(line_width),
          stroke_color(stroke_color),
          fill_color(fill_color) {}
    Size size;
    float line_width;
    Color stroke_color;
    Color fill_color;
    Gdiplus::GraphicsState state = 0;
  };

  // Return the top state.
  PainterState& top() { return states_.top(); }

  // The stack for all saved states.
  std::stack<PainterState> states_;

  Gdiplus::Graphics graphics_;

  // Current path.
  Gdiplus::GraphicsPath path_;
  // Current position, used when gdi+ does not have one.
  Gdiplus::PointF current_point_;
  // Whether gdi+ has a record of current point.
  bool use_gdi_current_point_ = true;

  Size size_;
  float scale_factor_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
