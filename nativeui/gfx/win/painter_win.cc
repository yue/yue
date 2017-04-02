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
                                 const nu::Rect& rect,
                                 const NativeTheme::ExtraParams& extra) {
  HDC hdc = GetHDC();
  State::GetCurrent()->GetNativeTheme()->Paint(
      part, hdc, state, rect, extra);
  ReleaseHDC(hdc);
}

void PainterWin::DrawFocusRect(const nu::Rect& rect) {
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
  MoveToPixel(ToFlooredPoint(ScalePoint(point, scale_factor_)));
}

void PainterWin::LineTo(const PointF& point) {
  LineToPixel(ToFlooredPoint(ScalePoint(point, scale_factor_)));
}

void PainterWin::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& ep) {
  BezierCurveToPixel(ToFlooredPoint(ScalePoint(cp1, scale_factor_)),
                     ToFlooredPoint(ScalePoint(cp2, scale_factor_)),
                     ToFlooredPoint(ScalePoint(ep, scale_factor_)));
}

void PainterWin::Rect(const RectF& rect) {
  RectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

void PainterWin::Clip() {
  graphics_.SetClip(&path_, Gdiplus::CombineModeIntersect);
}

void PainterWin::ClipRect(const RectF& rect) {
  ClipRectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
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

void PainterWin::MoveToPixel(const Point& point) {
  current_point_ = ToGdi(point);
}

void PainterWin::LineToPixel(const Point& point) {
  Gdiplus::Point p = ToGdi(point);
  path_.AddLine(current_point_, p);
  current_point_ = p;
}

void PainterWin::BezierCurveToPixel(const Point& cp1,
                                    const Point& cp2,
                                    const Point& end_point) {
  Gdiplus::Point ep = ToGdi(end_point);
  path_.AddBezier(current_point_, ToGdi(cp1), ToGdi(cp2), ep);
  current_point_ = ep;
}

void PainterWin::RectPixel(const nu::Rect& rect) {
  path_.AddRectangle(ToGdi(rect));
}

void PainterWin::ClipRectPixel(const nu::Rect& rect) {
  graphics_.SetClip(ToGdi(rect), Gdiplus::CombineModeIntersect);
}

void PainterWin::TranslatePixel(const Vector2d& offset) {
  graphics_.TranslateTransform(offset.x(), offset.y(),
                               Gdiplus::MatrixOrderAppend);
}

void PainterWin::StrokeRectPixel(const nu::Rect& rect) {
  Gdiplus::Pen pen(ToGdi(top().stroke_color), top().line_width);
  graphics_.DrawRectangle(&pen, ToGdi(rect));
}

void PainterWin::FillRectPixel(const nu::Rect& rect) {
  Gdiplus::SolidBrush brush(ToGdi(top().fill_color));
  graphics_.FillRectangle(&brush, ToGdi(rect));
}

void PainterWin::DrawColoredTextWithFlagsPixel(
    const String& text, Font* font, Color color, const nu::Rect& rect,
    int flags) {
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
