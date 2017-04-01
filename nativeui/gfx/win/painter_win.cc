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
    : hdc_(dc), scale_factor_(scale_factor) {
  // Initial state.
  states_.emplace(scale_factor, Color(), Color());
  // Enable using world transformation,
  ::SetGraphicsMode(hdc_, GM_ADVANCED);
}

PainterWin::~PainterWin() {
}

void PainterWin::DrawNativeTheme(NativeTheme::Part part,
                                 ControlState state,
                                 const Rect& rect,
                                 const NativeTheme::ExtraParams& extra) {
  State::GetCurrent()->GetNativeTheme()->Paint(
      part, hdc_, state, rect, extra);
}

void PainterWin::DrawFocusRect(const Rect& rect) {
  RECT r = rect.ToRECT();
  ::DrawFocusRect(hdc_, &r);
}

void PainterWin::Save() {
  states_.push(top());
  top().state = ::SaveDC(hdc_);
}

void PainterWin::Restore() {
  if (states_.size() == 1)
    return;
  ::RestoreDC(hdc_, top().state);
  states_.pop();
}

void PainterWin::BeginPath() {
  ::BeginPath(hdc_);
}

void PainterWin::ClosePath() {
  ::CloseFigure(hdc_);
  ::EndPath(hdc_);
}

void PainterWin::MoveTo(const PointF& point) {
  Point p = ToFlooredPoint(ScalePoint(point, scale_factor_));
  ::MoveToEx(hdc_, p.x(), p.y(), nullptr);
}

void PainterWin::LineTo(const PointF& point) {
  Point p = ToFlooredPoint(ScalePoint(point, scale_factor_));
  ::LineTo(hdc_, p.x(), p.y());
}

void PainterWin::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& ep) {
  POINT ps[3] = {
      ToFlooredPoint(ScalePoint(cp1, scale_factor_)).ToPOINT(),
      ToFlooredPoint(ScalePoint(cp2, scale_factor_)).ToPOINT(),
      ToFlooredPoint(ScalePoint(ep, scale_factor_)).ToPOINT(),
  };
  ::PolyBezierTo(hdc_, ps, 3);
}

void PainterWin::Clip() {
  ::SelectClipPath(hdc_, RGN_AND);
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
  base::win::ScopedGDIObject<HPEN> pen(
      ::CreatePen(PS_SOLID, top().line_width, top().stroke_color.ToCOLORREF()));
  base::win::ScopedSelectObject select_pen(hdc_, pen.get());
  ::StrokePath(hdc_);
}

void PainterWin::Fill() {
  base::win::ScopedGDIObject<HBRUSH> brush(
      ::CreateSolidBrush(top().stroke_color.ToCOLORREF()));
  base::win::ScopedSelectObject select_brush(hdc_, brush.get());
  ::FillPath(hdc_);
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
  base::win::ScopedRegion region(CreateRgnWithWorldTransform(rect));
  int clip_mode = RGN_COPY;
  if (mode == CombineMode::Intersect)
    clip_mode = RGN_AND;
  else if (mode == CombineMode::Union)
    clip_mode = RGN_OR;
  else if (mode == CombineMode::Exclude)
    clip_mode = RGN_DIFF;
  ::ExtSelectClipRgn(hdc_, region.get(), clip_mode);
}

void PainterWin::TranslatePixel(const Vector2d& offset) {
  XFORM xform = {0};
  xform.eM11 = 1.0;
  xform.eM22 = 1.0;
  xform.eDx = offset.x();
  xform.eDy = offset.y();
  ::ModifyWorldTransform(hdc_, &xform, MWT_LEFTMULTIPLY);
}

void PainterWin::StrokeRectPixel(const Rect& rect) {
  Gdiplus::Pen pen(ToGdi(top().stroke_color), top().line_width);
  Gdiplus::Graphics(hdc_).DrawRectangle(&pen, ToGdi(rect));
}

void PainterWin::FillRectPixel(const Rect& rect) {
  RECT r = rect.ToRECT();
  base::win::ScopedGDIObject<HBRUSH> brush(
      ::CreateSolidBrush(top().fill_color.ToCOLORREF()));
  ::FillRect(hdc_, &r, brush.get());
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
  Gdiplus::Graphics(hdc_).DrawString(
      text.c_str(), static_cast<int>(text.size()),
      font->GetNative(), ToGdi(RectF(rect)), &format, &brush);
}

HRGN PainterWin::CreateRgnWithWorldTransform(const Rect& rect) {
  XFORM xform = {0};
  if (!::GetWorldTransform(hdc_, &xform))
    LOG(ERROR) << "Unable to get current world transform";

  struct {
    RGNDATAHEADER rdh;
    RECT buffer;
  } rgn_data;
  rgn_data.buffer = rect.ToRECT();
  rgn_data.rdh.dwSize = sizeof(rgn_data.rdh);
  rgn_data.rdh.iType = RDH_RECTANGLES;
  rgn_data.rdh.nCount = 1;
  rgn_data.rdh.nRgnSize = sizeof(rgn_data);
  rgn_data.rdh.rcBound = rgn_data.buffer;
  return ::ExtCreateRegion(&xform, sizeof(rgn_data),
                           reinterpret_cast<RGNDATA*>(&rgn_data));
}

}  // namespace nu
