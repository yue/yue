// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/painter_win.h"

#include "base/win/scoped_gdi_object.h"
#include "nativeui/gfx/win/pen_win.h"

namespace nu {

PainterWin::PainterState::PainterState(const Vector2d& origin,
                                       Gdiplus::GraphicsContainer&& container)
    : origin(origin), container(container) {
}

PainterWin::PainterWin(HDC dc) : graphics_(dc) {
}

PainterWin::~PainterWin() {
}

HDC PainterWin::GetHDC() {
  // Get the clip region of graphics.
  Gdiplus::Rect clip;
  graphics_.GetVisibleClipBounds(&clip);
  base::win::ScopedRegion region(::CreateRectRgn(
      clip.GetLeft(), clip.GetTop(), clip.GetRight(), clip.GetBottom()));

  // Apply current clip region to the returned HDC.
  HDC dc = graphics_.GetHDC();
  ::SelectClipRgn(dc, region.get());
  return dc;
}

void PainterWin::ReleaseHDC(HDC dc) {
  graphics_.ReleaseHDC(dc);
}

void PainterWin::Save() {
  states_.emplace(origin(), graphics_.BeginContainer());
}

void PainterWin::Restore() {
  if (states_.empty())
    return;
  graphics_.EndContainer(states_.top().container);
  states_.pop();
}

void PainterWin::ClipRect(const Rect& rect, CombineMode mode) {
  Gdiplus::CombineMode cm;
  switch (mode) {
    case CombineMode::Replace   : cm = Gdiplus::CombineModeReplace;   break;
    case CombineMode::Intersect : cm = Gdiplus::CombineModeIntersect; break;
    case CombineMode::Union     : cm = Gdiplus::CombineModeUnion;     break;
    case CombineMode::Exclude   : cm = Gdiplus::CombineModeExclude;   break;
    default: cm = Gdiplus::CombineModeReplace;
  }
  graphics_.SetClip(ToGdi(rect + origin()), cm);
}

void PainterWin::Translate(const Vector2d& offset) {
  origin() += offset;
}

void PainterWin::DrawRect(const Rect& rect, Color color) {
  Gdiplus::Pen pen(ToGdi(color));
  graphics_.DrawRectangle(&pen, ToGdi(rect + origin()));
}

void PainterWin::DrawRect(const Rect& rect, Pen* pen) {
  graphics_.DrawRectangle(static_cast<PenWin*>(pen)->pen(),
                          ToGdi(rect + origin()));
}

void PainterWin::FillRect(const Rect& rect, Color color) {
  Gdiplus::SolidBrush brush(ToGdi(color));
  graphics_.FillRectangle(&brush, ToGdi(rect + origin()));
}

void PainterWin::DrawStringWithFlags(const String& text,
                                     Font font,
                                     Color color,
                                     const Rect& rect,
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
  graphics_.DrawString(text.c_str(), static_cast<int>(text.size()),
                       font.GetNativeFont(), ToGdi(RectF(rect + origin())),
                       &format, &brush);
}

// static
std::unique_ptr<Painter> Painter::CreateFromHDC(HDC dc) {
  return std::unique_ptr<Painter>(new PainterWin(dc));
}

}  // namespace nu
