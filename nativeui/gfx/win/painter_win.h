// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PAINTER_WIN_H_
#define NATIVEUI_GFX_WIN_PAINTER_WIN_H_

#include <stack>

#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/gfx/win/native_theme.h"

namespace nu {

class PainterWin : public Painter {
 public:
  PainterWin(HDC hdc, float scale_factor);
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
  void ArcTo(const PointF& cp1, const PointF& cp2, float radius) override;
  void Arc(const PointF& point, float radius, float sa, float ea) override;
  void Rect(const RectF& rect) override;
  void Clip() override;
  void ClipRect(const RectF& rect) override;
  void Translate(const Vector2dF& offset) override;
  void SetColor(Color color) override;
  void SetStrokeColor(Color color) override;
  void SetFillColor(Color color) override;
  void SetLineWidth(float width) override;
  void Stroke() override;
  void Fill() override;
  void StrokeRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawColoredTextWithFlags(
      const String& text, Font* font, Color color, const RectF& rect,
      int flags) override;

  // The pixel versions.
  void MoveToPixel(const PointF& point);
  void LineToPixel(const PointF& point);
  void BezierCurveToPixel(const PointF& cp1,
                          const PointF& cp2,
                          const PointF& ep);
  void ArcToPixel(const PointF& cp1, const PointF& cp2, float radius);
  void ArcPixel(const PointF& point, float r, float sa, float ea,
                bool anticlockwise);
  void RectPixel(const RectF& rect);
  void ClipRectPixel(const nu::Rect& rect);
  void TranslatePixel(const Vector2d& offset);
  void StrokeRectPixel(const nu::Rect& rect);
  void FillRectPixel(const nu::Rect& rect);
  void DrawColoredTextWithFlagsPixel(
      const String& text, Font* font, Color color, const nu::Rect& rect,
      int flags);

 private:
  // Get current point.
  bool GetCurrentPoint(Gdiplus::PointF* point);

  // Receive the HDC that can be painted on.
  HDC GetHDC();
  void ReleaseHDC(HDC dc);

  // The saved state.
  struct PainterState {
    PainterState(float line_width, Color stroke_color, Color fill_color)
        : line_width(line_width),
          stroke_color(stroke_color),
          fill_color(fill_color) {}
    float line_width;
    Color stroke_color;
    Color fill_color;
    Gdiplus::GraphicsState state = 0;
  };

  // Return the top state.
  PainterState& top() { return states_.top(); }

  // The stack for all saved states.
  std::stack<PainterState> states_;

  // Weak ref to the original HDC, should only be used internally.
  HDC hdc_;

  // Current path.
  Gdiplus::GraphicsPath path_;
  // Current position, used when gdi+ does not have one.
  Gdiplus::PointF current_point_;
  // Whether gdi+ has a record of current point.
  bool use_gdi_current_point_;

  float scale_factor_;
  Gdiplus::Graphics graphics_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
