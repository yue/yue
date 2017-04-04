// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2005, 2007 Apple Inc.
// Copyright 2004, 2005, 2006 Nikolas Zimmermann <wildfox@kde.org>
//           2004, 2005, 2006 Rob Buis <buis@kde.org>
//           2007 Alp Toker <alp@atoker.com>
//           2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
//           2008 Dirk Schulze <krit@webkit.org>
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/painter_win.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <memory>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_select_object.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/vector2d_conversions.h"
#include "nativeui/state.h"

namespace nu {

namespace {

inline float AreaOfTriangleFormedByPoints(const PointF& p1,
                                          const PointF& p2,
                                          const PointF& p3) {
  return p1.x() * (p2.y() - p3.y()) +
         p2.x() * (p3.y() - p1.y()) +
         p3.x() * (p1.y() - p2.y());
}

}  // namespace


PainterWin::PainterWin(HDC hdc, float scale_factor)
    : use_gdi_current_point_(true),
      scale_factor_(scale_factor),
      graphics_(hdc) {
  // Use high quality rendering.
  graphics_.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
  graphics_.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
  graphics_.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
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
  use_gdi_current_point_ = true;
  path_.Reset();
  path_.StartFigure();
}

void PainterWin::ClosePath() {
  use_gdi_current_point_ = true;
  path_.CloseAllFigures();
}

void PainterWin::MoveTo(const PointF& point) {
  MoveToPixel(ScalePoint(point, scale_factor_));
}

void PainterWin::LineTo(const PointF& point) {
  LineToPixel(ScalePoint(point, scale_factor_));
}

void PainterWin::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& ep) {
  BezierCurveToPixel(ScalePoint(cp1, scale_factor_),
                     ScalePoint(cp2, scale_factor_),
                     ScalePoint(ep, scale_factor_));
}

void PainterWin::ArcTo(const PointF& cp1, const PointF& cp2, float radius) {
  ArcToPixel(ScalePoint(cp1, scale_factor_),
             ScalePoint(cp2, scale_factor_),
             radius * scale_factor_);
}

void PainterWin::Arc(const PointF& point, float radius, float sa, float ea) {
  ArcPixel(ScalePoint(point, scale_factor_), radius * scale_factor_, sa, ea,
           false);
}

void PainterWin::Rect(const RectF& rect) {
  RectPixel(ScaleRect(rect, scale_factor_));
}

void PainterWin::Clip() {
  graphics_.SetClip(&path_, Gdiplus::CombineModeIntersect);
  path_.Reset();
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

void PainterWin::SetStrokeColor(Color color) {
  top().stroke_color = color;
}

void PainterWin::SetFillColor(Color color) {
  top().fill_color = color;
}

void PainterWin::SetLineWidth(float width) {
  top().line_width = width;
}

void PainterWin::Stroke() {
  Gdiplus::Pen pen(ToGdi(top().stroke_color), top().line_width);
  graphics_.DrawPath(&pen, &path_);
  path_.Reset();
}

void PainterWin::Fill() {
  Gdiplus::SolidBrush brush(ToGdi(top().fill_color));
  graphics_.FillPath(&brush, &path_);
  path_.Reset();
}

void PainterWin::StrokeRect(const RectF& rect) {
  StrokeRectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

void PainterWin::FillRect(const RectF& rect) {
  FillRectPixel(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

SizeF PainterWin::MeasureText(base::StringPiece text, Font* font) {
  Gdiplus::RectF rect;
  base::string16 wtext(base::UTF8ToUTF16(text));
  graphics_.MeasureString(wtext.c_str(), static_cast<int>(wtext.length()),
                          font->GetNative(), Gdiplus::PointF(0., 0.), &rect);
  return ScaleSize(SizeF(rect.Width, rect.Height), 1.0f / scale_factor_);
}

void PainterWin::DrawColoredTextWithFlags(
    base::StringPiece text, Font* font, Color color, const RectF& rect, int flags) {
  DrawColoredTextWithFlagsPixel(
      text, font, color, ToEnclosingRect(ScaleRect(rect, scale_factor_)),
      flags);
}

void PainterWin::MoveToPixel(const PointF& point) {
  path_.StartFigure();
  use_gdi_current_point_ = false;
  current_point_ = ToGdi(point);
}

void PainterWin::LineToPixel(const PointF& point) {
  Gdiplus::PointF start;
  if (!GetCurrentPoint(&start)) {
    MoveToPixel(point);
    return;
  }
  use_gdi_current_point_ = true;
  path_.AddLine(start, ToGdi(point));
}

void PainterWin::BezierCurveToPixel(const PointF& cp1,
                                    const PointF& cp2,
                                    const PointF& ep) {
  Gdiplus::PointF start;
  if (!GetCurrentPoint(&start)) {
    MoveToPixel(ep);
    return;
  }
  use_gdi_current_point_ = true;
  path_.AddBezier(start, ToGdi(cp1), ToGdi(cp2), ToGdi(ep));
}

void PainterWin::ArcToPixel(const PointF& p1, const PointF& p2, float radius) {
  // Current position.
  Gdiplus::PointF tp;
  GetCurrentPoint(&tp);
  PointF p0(tp.X, tp.Y);

  // Draw only a straight line to p1 if any of the points are equal or the
  // radius is zero or the points are collinear (triangle that the points form
  // has area of zero value).
  if (p0 == p1 || p1 == p2 || radius == 0 ||
      AreaOfTriangleFormedByPoints(p0, p1, p2) == 0) {
    LineToPixel(p1);
    return;
  }

  PointF p1p0(p0.x() - p1.x(), p0.y() - p1.y());
  PointF p1p2(p2.x() - p1.x(), p2.y() - p1.y());
  float p1p0_length = sqrtf(p1p0.x() * p1p0.x() + p1p0.y() * p1p0.y());
  float p1p2_length = sqrtf(p1p2.x() * p1p2.x() + p1p2.y() * p1p2.y());

  double cos_phi = (p1p0.x() * p1p2.x() + p1p0.y() * p1p2.y()) /
                   (p1p0_length * p1p2_length);
  // All points on a line logic.
  if (cos_phi == -1) {
    LineToPixel(p1);
    return;
  }
  if (cos_phi == 1) {
    // Add infinite far away point.
    unsigned int max_length = 65535;
    double factor_max = max_length / p1p0_length;
    PointF ep(p0.x() + factor_max * p1p0.x(), p0.y() + factor_max * p1p0.y());
    LineToPixel(ep);
    return;
  }

  float tangent = radius / tan(acos(cos_phi) / 2);
  float factor_p1p0 = tangent / p1p0_length;
  PointF t_p1p0(p1.x() + factor_p1p0 * p1p0.x(),
                p1.y() + factor_p1p0 * p1p0.y());

  PointF orth_p1p0(p1p0.y(), -p1p0.x());
  float orth_p1p0_length = sqrt(orth_p1p0.x() * orth_p1p0.x() +
                                orth_p1p0.y() * orth_p1p0.y());
  float factor_ra = radius / orth_p1p0_length;

  // Angle between orth_p1p0 and p1p2 to get the right vector orthographic to
  // p1p0.
  double cos_alpha = (orth_p1p0.x() * p1p2.x() + orth_p1p0.y() * p1p2.y()) /
                     (orth_p1p0_length * p1p2_length);
  if (cos_alpha < 0.f)
    orth_p1p0 = PointF(-orth_p1p0.x(), -orth_p1p0.y());

  PointF p(t_p1p0.x() + factor_ra * orth_p1p0.x(),
           t_p1p0.y() + factor_ra * orth_p1p0.y());

  // Calculate angles for addArc.
  orth_p1p0 = PointF(-orth_p1p0.x(), -orth_p1p0.y());
  float sa = acos(orth_p1p0.x() / orth_p1p0_length);
  if (orth_p1p0.y() < 0.f)
    sa = 2 * M_PI - sa;

  // Anti-clockwise logic.
  bool anticlockwise = false;

  float factor_p1p2 = tangent / p1p2_length;
  PointF t_p1p2(p1.x() + factor_p1p2 * p1p2.x(),
                p1.y() + factor_p1p2 * p1p2.y());
  PointF orth_p1p2(t_p1p2.x() - p.x(), t_p1p2.y() - p.y());
  float orth_p1p2_length = sqrtf(orth_p1p2.x() * orth_p1p2.x() +
                                 orth_p1p2.y() * orth_p1p2.y());
  float ea = acos(orth_p1p2.x() / orth_p1p2_length);
  if (orth_p1p2.y() < 0)
    ea = 2 * M_PI - ea;
  if ((sa > ea) && ((sa - ea) < M_PI))
    anticlockwise = true;
  if ((sa < ea) && ((ea - sa) > M_PI))
    anticlockwise = true;

  ArcPixel(p, radius, sa, ea, anticlockwise);
}

void PainterWin::ArcPixel(const PointF& p, float radius, float sa, float ea,
                          bool anticlockwise) {
  // Normalize the angle.
  float angle;
  if (anticlockwise) {
    if (ea > sa) {
      while (ea >= sa)
        ea -= 2.0f * static_cast<float>(M_PI);
    }
    angle = sa - ea;
  } else {
    if (ea < sa) {
      while (ea <= sa)
        ea += 2.0f * static_cast<float>(M_PI);
    }
    angle = ea - sa;
  }

  // Should draw a line connecting previous point.
  LineToPixel(PointF(p.x() + radius * cos(sa), p.y() + radius * sin(sa)));

  // AddArc would connect to the previous point under some cases, but not
  // always, we have to disable this behavior via starting a sub-path.
  path_.StartFigure();

  use_gdi_current_point_ = true;
  path_.AddArc(p.x() - radius, p.y() - radius, 2.0f * radius, 2.0f * radius,
               sa / M_PI * 180.0f,
               (anticlockwise ? -angle : angle) / M_PI * 180.0f);
}

void PainterWin::RectPixel(const RectF& rect) {
  path_.AddRectangle(ToGdi(rect));
  // Drawing rectangle should update current point.
  MoveToPixel(rect.origin());
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
  // Should clear current path.
  use_gdi_current_point_ = true;
  path_.Reset();
}

void PainterWin::FillRectPixel(const nu::Rect& rect) {
  Gdiplus::SolidBrush brush(ToGdi(top().fill_color));
  graphics_.FillRectangle(&brush, ToGdi(rect));
  // Should clear current path.
  use_gdi_current_point_ = true;
  path_.Reset();
}

bool PainterWin::GetCurrentPoint(Gdiplus::PointF* point) {
  if (use_gdi_current_point_) {
    Gdiplus::Status status = path_.GetLastPoint(point);
    if (status != Gdiplus::Ok)
      return false;
  } else {
    *point = current_point_;
  }
  return true;
}

void PainterWin::DrawColoredTextWithFlagsPixel(
    base::StringPiece text, Font* font, Color color, const nu::Rect& rect,
    int flags) {
  DrawColoredTextWithFlagsPixel(base::UTF8ToUTF16(text), font, color, rect,
                                flags);
}

void PainterWin::DrawColoredTextWithFlagsPixel(
    const base::string16& text, Font* font, Color color, const nu::Rect& rect,
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
