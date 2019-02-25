// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PAINTER_WIN_H_
#define NATIVEUI_GFX_WIN_PAINTER_WIN_H_

#include <wrl/client.h>

#include <stack>
#include <string>

#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/gfx/win/native_theme.h"

namespace nu {

class DWriteTextRenderer;

class PainterWin : public Painter {
 public:
  // Paint on the HDC.
  PainterWin(HDC hdc, const Size& size, float scale_factor);
  // Paint on the bitmap.
  PainterWin(NativeBitmap bitmap, float scale_factor);
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
  void StrokeRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawImage(Image* image, const RectF& rect) override;
  void DrawImageFromRect(Image* image, const RectF& src,
                         const RectF& dest) override;
  void DrawCanvas(Canvas* canvas, const RectF& rect) override;
  void DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                          const RectF& dest) override;
  void DrawAttributedText(AttributedText* text, const RectF& rect) override;
  TextMetrics MeasureText(const std::string& text, float width,
                          const TextAttributes& attributes) override;
  // Unlike other platforms that drawing attributed text is basically the same
  // with drawing normal text, on Windows attributed text is drawn with the
  // DirectWrite and we have to create a bunch of COM objects everytime.
  //
  // So we still use GDI+ for drawing simple text for performance.
  void DrawText(const std::string& text, const RectF& rect,
                const TextAttributes& attributes) override;

  // The pixel versions.
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
  void DrawTextPixel(const base::string16& text, const nu::Rect& rect,
                     const TextAttributes& attributes);
  void DrawTextPixel(const base::string16& text, const PointF& point,
                     const TextAttributes& attributes);

 private:
  // Used for common initialization.
  void Initialize(float scale_factor);

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
          fill_color(fill_color),
          state(0),
          matrix(1.f, 0, 0, 1.f, 0, 0) {}
    float line_width;
    Color stroke_color;
    Color fill_color;
    Gdiplus::GraphicsState state;

    // D2D1 uses DIP metrics, so under high DPI the transformation of GDI+
    // would be wrong since the latter uses pixels, to work around this we
    // save a copy of matrix that uses DIP metrics.
    D2D1::Matrix3x2F matrix;
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
  bool use_gdi_current_point_;

  Size size_;
  float scale_factor_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
