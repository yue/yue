// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/painter_win.h"

#include <memory>

#include "base/win/scoped_gdi_object.h"
#include "nativeui/gfx/geometry/vector2d_conversions.h"
#include "nativeui/state.h"

namespace nu {

PainterWin::PainterWin(HDC dc, float scale_factor)
    : hdc_(dc), scale_factor_(scale_factor) {
  // Initial state.
  states_.emplace(Vector2dF(), scale_factor, Color());
}

PainterWin::~PainterWin() {
}

void PainterWin::DrawNativeTheme(NativeTheme::Part part,
                                 ControlState state,
                                 const Rect& rect,
                                 const NativeTheme::ExtraParams& extra) {
  State::GetCurrent()->GetNativeTheme()->Paint(
      part, hdc_, state, Translated(rect), extra);
}

void PainterWin::DrawFocusRect(const Rect& rect) {
  RECT r = Translated(rect).ToRECT();
  ::DrawFocusRect(hdc_, &r);
}

void PainterWin::Save() {
  top().state = ::SaveDC(hdc_);
  states_.push(top());
}

void PainterWin::Restore() {
  if (states_.size() == 1)
    return;
  ::RestoreDC(hdc_, top().state);
  states_.pop();
}

void PainterWin::ClipRect(const RectF& rect, CombineMode mode) {
  ClipPixelRect(ScaleRect(rect, scale_factor_), mode);
}

void PainterWin::Translate(const Vector2dF& offset) {
  TranslatePixel(ScaleVector2d(offset, scale_factor_));
}

void PainterWin::SetColor(Color color) {
  top().color = color;
}

void PainterWin::SetLineWidth(float width) {
  top().line_width = width;
}

void PainterWin::DrawRect(const RectF& rect) {
  DrawPixelRect(ScaleRect(rect, scale_factor_));
}

void PainterWin::FillRect(const RectF& rect) {
  FillPixelRect(ScaleRect(rect, scale_factor_));
}

void PainterWin::DrawColoredTextWithFlags(
    const String& text, Font* font, Color color, const RectF& rect, int flags) {
  DrawColoredTextPixelWithFlags(text, font, color,
                                ScaleRect(rect, scale_factor_), flags);
}

void PainterWin::ClipPixelRect(const RectF& rect, CombineMode mode) {
  RectF r(Translated(rect));
  base::win::ScopedRegion region(
      ::CreateRectRgn(r.x(), r.y(), r.right(), r.bottom()));
  int clip_mode = RGN_COPY;
  if (mode == CombineMode::Intersect)
    clip_mode = RGN_AND;
  else if (mode == CombineMode::Union)
    clip_mode = RGN_OR;
  else if (mode == CombineMode::Exclude)
    clip_mode = RGN_DIFF;
  ::ExtSelectClipRgn(hdc_, region.get(), clip_mode);
}

void PainterWin::TranslatePixel(const Vector2dF& offset) {
  top().origin += offset;
}

void PainterWin::DrawPixelRect(const RectF& rect) {
  Gdiplus::Pen pen(ToGdi(top().color), top().line_width);
  Gdiplus::Graphics(hdc_).DrawRectangle(&pen, ToGdi(Translated(rect)));
}

void PainterWin::FillPixelRect(const RectF& rect) {
  Gdiplus::SolidBrush brush(ToGdi(top().color));
  Gdiplus::Graphics(hdc_).FillRectangle(&brush, ToGdi(Translated(rect)));
}

void PainterWin::DrawColoredTextPixelWithFlags(
    const String& text, Font* font, Color color, const RectF& rect, int flags) {
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
      font->GetNative(), ToGdi(Translated(rect)),
      &format, &brush);
}

Rect PainterWin::Translated(const Rect& rect) {
  return rect + ToCeiledVector2d(top().origin);
}

RectF PainterWin::Translated(const RectF& rect) {
  return rect + top().origin;
}

}  // namespace nu
