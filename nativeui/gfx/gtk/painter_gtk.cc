// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
#define NATIVEUI_GFX_GTK_PAINTER_GTK_CC_

#include "nativeui/gfx/gtk/painter_gtk.h"

#include <gtk/gtk.h>

namespace nu {

PainterGtk::PainterGtk(cairo_t* context) : context_(context) {
}

PainterGtk::~PainterGtk() {
}

void PainterGtk::Save() {
  cairo_save(context_);
}

void PainterGtk::Restore() {
  cairo_restore(context_);
}

void PainterGtk::BeginPath() {
  cairo_new_path(context_);
}

void PainterGtk::ClosePath() {
  cairo_close_path(context_);
}

void PainterGtk::MoveTo(const PointF& point) {
  cairo_move_to(context_, point.x(), point.y());
}

void PainterGtk::LineTo(const PointF& point) {
  cairo_line_to(context_, point.x(), point.y());
}

void PainterGtk::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& ep) {
  cairo_curve_to(
      context_, cp1.x(), cp1.y(), cp2.x(), cp2.y(), ep.x(), ep.y());
}

void PainterGtk::Rect(const RectF& rect) {
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
}

void PainterGtk::Clip() {
  cairo_clip(context_);
}

void PainterGtk::ClipRect(const RectF& rect) {
  cairo_new_path(context_);
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_clip(context_);
}

void PainterGtk::Translate(const Vector2dF& offset) {
  cairo_translate(context_, offset.x(), offset.y());
}

void PainterGtk::SetColor(Color color) {
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);
}

void PainterGtk::SetLineWidth(float width) {
  cairo_set_line_width(context_, width);
}

void PainterGtk::Stroke() {
  cairo_stroke(context_);
}

void PainterGtk::Fill() {
  cairo_fill(context_);
}

void PainterGtk::StrokeRect(const RectF& rect) {
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_stroke(context_);
}

void PainterGtk::FillRect(const RectF& rect) {
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_fill(context_);
}

void PainterGtk::DrawColoredTextWithFlags(
    const String& text, Font* font, Color color, const RectF& rect, int flags) {
  PangoLayout* layout = pango_cairo_create_layout(context_);
  pango_layout_set_font_description(layout, font->GetNative());
  cairo_save(context_);

  // Text size.
  int width, height;
  pango_layout_set_text(layout, text.c_str(), text.length());
  pango_layout_get_pixel_size(layout, &width, &height);

  int x = rect.x(), y = rect.y();
  // Vertical center.
  y += (rect.height() - height) / 2;

  // Horizontal alignment.
  if (flags & TextAlignRight)
    x += rect.width() - width;
  else if (flags & TextAlignCenter)
    x += (rect.width() - width) / 2;

  // Apply the color.
  SetColor(color);

  // Draw text.
  cairo_move_to(context_, x, y);
  pango_cairo_show_layout(context_, layout);

  cairo_restore(context_);
  g_object_unref(layout);
}

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
