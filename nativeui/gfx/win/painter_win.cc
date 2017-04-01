// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/painter_win.h"

#include <memory>

#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_select_object.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/vector2d_conversions.h"
#include "nativeui/state.h"

namespace nu {

PainterWin::PainterWin(HDC dc, float scale_factor)
    : hdc_(dc), scale_factor_(scale_factor), graphics_(hdc_) {
  // Initial state.
  states_.emplace(scale_factor, Color(), Color());
}

PainterWin::~PainterWin() {
}

void PainterWin::DrawNativeTheme(NativeTheme::Part part,
                                 ControlState state,
                                 const Rect& rect,
                                 const NativeTheme::ExtraParams& extra) {
  HDC hdc = GetHDC();
  State::GetCurrent()->GetNativeTheme()->Paint(
      part, hdc, state, rect, extra);
  ReleaseHDC(hdc);
}

void PainterWin::DrawFocusRect(const Rect& rect) {
  RECT r = rect.ToRECT();
  HDC hdc = GetHDC();
  ::DrawFocusRect(hdc, &r);
  ReleaseHDC(hdc);
}

void PainterWin::Save() {
  states_.push(top());
  top().state = graphics_.Save();
}

void PainterWin::Restore() {
  if (states_.size() == 1)
    return;
  graphics_.Restore(top().state);
  states_.pop();
}

void PainterWin::BeginPath() {
  current_point_.X = current_point_.Y = 0;
  path_.Reset();
  path_.StartFigure();
}

void PainterWin::ClosePath() {
  current_point_.X = current_point_.Y = 0;
  path_.CloseFigure();
}

void PainterWin::MoveTo(const PointF& point) {
  current_point_ = ToGdi(ScalePoint(point, scale_factor_));
}

void PainterWin::LineTo(const PointF& point) {
  Gdiplus::PointF p = ToGdi(ScalePoint(point, scale_factor_));
  path_.AddLine(current_point_, p);
  current_point_ = p;
}

void PainterWin::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& end_point) {
  Gdiplus::PointF ep = ToGdi(ScalePoint(end_point, scale_factor_));
  path_.AddBezier(current_point_,
                  ToGdi(ScalePoint(cp1, scale_factor_)),
                  ToGdi(ScalePoint(cp2, scale_factor_)),
                  ep);
  current_point_ = ep;
}

void PainterWin::Clip() {
  graphics_.SetClip(&path_, Gdiplus::CombineModeIntersect);
}

void PainterWin::ClipRect(const RectF& rect, CombineMode mode) {
  ClipRectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)), mode);
}

void PainterWin::Translate(const Vector2dF& offset) {
  TranslatePixel(ToFlooredVector2d(ScaleVector2d(offset, scale_factor_)));
}

void PainterWin::SetColor(Color color) {
  top().stroke_color = color;
  top().fill_color = color;
}

void PainterWin::SetLineWidth(float width) {
  top().line_width = width;
}

void PainterWin::Stroke() {
  Gdiplus::Pen pen(ToGdi(top().stroke_color), top().line_width);
  graphics_.DrawPath(&pen, &path_);
}

void PainterWin::Fill() {
  Gdiplus::SolidBrush brush(ToGdi(top().fill_color));
  graphics_.FillPath(&brush, &path_);
}

void PainterWin::StrokeRect(const RectF& rect) {
  StrokeRectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

void PainterWin::FillRect(const RectF& rect) {
  FillRectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

void PainterWin::DrawColoredTextWithFlags(
    const String& text, Font* font, Color color, const RectF& rect, int flags) {
  DrawColoredTextWithFlagsPixel(
      text, font, color, ToEnclosingRect(ScaleRect(rect, scale_factor_)),
      flags);
}

void PainterWin::ClipRectPixel(const Rect& rect, CombineMode mode) {
  Gdiplus::CombineMode cm;
  switch (mode) {
    case CombineMode::Replace   : cm = Gdiplus::CombineModeReplace;   break;
    case CombineMode::Intersect : cm = Gdiplus::CombineModeIntersect; break;
    case CombineMode::Union     : cm = Gdiplus::CombineModeUnion;     break;
    case CombineMode::Exclude   : cm = Gdiplus::CombineModeExclude;   break;
    default: cm = Gdiplus::CombineModeReplace;
  }
  graphics_.SetClip(ToGdi(rect), cm);
}

void PainterWin::TranslatePixel(const Vector2d& offset) {
  graphics_.TranslateTransform(offset.x(), offset.y(),
                               Gdiplus::MatrixOrderAppend);
}

void PainterWin::StrokeRectPixel(const Rect& rect) {
  Gdiplus::Pen pen(ToGdi(top().stroke_color), top().line_width);
  graphics_.DrawRectangle(&pen, ToGdi(rect));
}

void PainterWin::FillRectPixel(const Rect& rect) {
  Gdiplus::SolidBrush brush(ToGdi(top().fill_color));
  graphics_.FillRectangle(&brush, ToGdi(rect));
}

void PainterWin::DrawColoredTextWithFlagsPixel(
    const String& text, Font* font, Color color, const Rect& rect, int flags) {
  Gdiplus::SolidBrush brush(ToGdi(color));
  Gdiplus::StringFormat format;
  format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
  if (flags & TextAlignLeft)
    format.SetAlignment(Gdiplus::StringAlignmentNear);
  else if (flags & TextAlignCenter)
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
  else if (flags & TextAlignRight)
    format.SetAlignment(Gdiplus::StringAlignmentFar);
  graphics_.DrawString(
      text.c_str(), static_cast<int>(text.size()),
      font->GetNative(), ToGdi(RectF(rect)), &format, &brush);
}

HDC PainterWin::GetHDC() {
  // Get the clip region of graphics.
  Gdiplus::Region clip;
  graphics_.GetClip(&clip);
  base::win::ScopedRegion region(clip.GetHRGN(&graphics_));
  // Get the transform of graphics.
  Gdiplus::Matrix matrix;
  graphics_.GetTransform(&matrix);
  XFORM xform;
  matrix.GetElements(reinterpret_cast<float*>(&xform));

  // Apply current clip region to the returned HDC.
  HDC hdc = graphics_.GetHDC();
  ::SelectClipRgn(hdc, region.get());
  // Apply the world transform to HDC.
  ::SetGraphicsMode(hdc, GM_ADVANCED);
  ::SetWorldTransform(hdc, &xform);
  return hdc;
}

void PainterWin::ReleaseHDC(HDC hdc) {
  // Change the world transform back, otherwise the world transform of GDI+
  // would be affected.
  XFORM xform = {0};
  xform.eM11 = 1.;
  xform.eM22 = 1.;
  ::SetWorldTransform(hdc, &xform);
  // Return HDC.
  graphics_.ReleaseHDC(hdc);
}

}  // namespace nu
