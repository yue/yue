// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/painter_win.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <memory>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_select_object.h"
#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/canvas.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/point_conversions.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/vector2d_conversions.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/win/attributed_text_win.h"
#include "nativeui/gfx/win/double_buffer.h"
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
    : graphics_(hdc) {
  Initialize(scale_factor);
}

PainterWin::~PainterWin() {}

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

void PainterWin::Rotate(float angle) {
  graphics_.RotateTransform(angle / M_PI * 180.0f);
}

void PainterWin::Scale(const Vector2dF& scale) {
  graphics_.ScaleTransform(scale.x(), scale.y());
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

void PainterWin::DrawImage(Image* image, const RectF& rect) {
  graphics_.DrawImage(image->GetNative(),
                      ToGdi(ScaleRect(rect, scale_factor_)));
}

void PainterWin::DrawImageFromRect(Image* image, const RectF& src,
                                   const RectF& dest) {
  RectF ps = ScaleRect(src, image->GetScaleFactor());
  graphics_.DrawImage(image->GetNative(),
                      ToGdi(ScaleRect(dest, scale_factor_)),
                      ps.x(), ps.y(), ps.width(), ps.height(),
                      Gdiplus::UnitPixel);
}

void PainterWin::DrawCanvas(Canvas* canvas, const RectF& rect) {
  std::unique_ptr<Gdiplus::Bitmap> bitmap =
      canvas->GetBitmap()->GetGdiplusBitmap();
  graphics_.DrawImage(bitmap.get(), ToGdi(ScaleRect(rect, scale_factor_)));
}

void PainterWin::DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                                    const RectF& dest) {
  std::unique_ptr<Gdiplus::Bitmap> bitmap =
      canvas->GetBitmap()->GetGdiplusBitmap();
  RectF ps = ScaleRect(src, canvas->GetScaleFactor());
  graphics_.DrawImage(bitmap.get(),
                      ToGdi(ScaleRect(dest, scale_factor_)),
                      ps.x(), ps.y(), ps.width(), ps.height(),
                      Gdiplus::UnitPixel);
}

TextMetrics PainterWin::MeasureText(const std::string& text, float width,
                                    const TextAttributes& attributes) {
  if (width >= 0)
    width = width * scale_factor_;
  else
    width = FLT_MAX;
  base::string16 wtext(base::UTF8ToUTF16(text));
  Gdiplus::RectF rect;
  Gdiplus::StringFormat format;
  graphics_.MeasureString(wtext.c_str(), static_cast<int>(wtext.length()),
                          attributes.font->GetNative(),
                          Gdiplus::RectF(0.f, 0.f, width, FLT_MAX),
                          &format, &rect, nullptr, nullptr);
  return { ScaleSize(SizeF(rect.Width, rect.Height), 1.0f / scale_factor_) };
}

void PainterWin::DrawAttributedText(AttributedText* text, const RectF& rect) {
  AttributedTextImpl* str = text->GetNative();
  graphics_.DrawString(
      str->text.data(), static_cast<int>(str->text.size()),
      str->font->GetNative(),
      ToGdi(ScaleRect(rect, scale_factor_)),
      &str->format, str->brush.get());
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

void PainterWin::DrawTextPixel(const base::string16& text, const nu::Rect& rect,
                               const TextAttributes& attributes) {
  Gdiplus::SolidBrush brush(ToGdi(attributes.color));
  Gdiplus::StringFormat format;
  format.SetAlignment(ToGdi(attributes.align));
  format.SetLineAlignment(ToGdi(attributes.valign));
  if (!attributes.wrap)
    format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
  if (attributes.ellipsis)
    format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
  graphics_.DrawString(
      text.c_str(), static_cast<int>(text.size()),
      attributes.font->GetNative(), ToGdi(RectF(rect)), &format, &brush);
}

void PainterWin::DrawTextPixel(const base::string16& text, const PointF& point,
                               const TextAttributes& attributes) {
  Gdiplus::SolidBrush brush(ToGdi(attributes.color));
  graphics_.DrawString(
      text.c_str(), static_cast<int>(text.size()),
      attributes.font->GetNative(), ToGdi(point), &brush);
}

void PainterWin::Initialize(float scale_factor) {
  use_gdi_current_point_ = true;
  scale_factor_ = scale_factor;

  // Use high quality rendering.
  graphics_.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
  graphics_.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
  graphics_.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
  graphics_.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
  // Initial state.
  states_.emplace(scale_factor, Color(), Color());
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
