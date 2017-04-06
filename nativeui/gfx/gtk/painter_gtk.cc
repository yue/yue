// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2005, 2007 Apple Inc.
// Copyright 2004, 2005, 2006 Nikolas Zimmermann <wildfox@kde.org>
//           2004, 2005, 2006 Rob Buis <buis@kde.org>
//           2007 Alp Toker <alp@atoker.com>
//           2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
//           2008 Dirk Schulze <krit@webkit.org>
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
#define NATIVEUI_GFX_GTK_PAINTER_GTK_CC_

#include "nativeui/gfx/gtk/painter_gtk.h"

#include <math.h>

#include <gtk/gtk.h>

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/image.h"

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

PainterGtk::PainterGtk(cairo_t* context) : context_(context) {
  // Initial state.
  states_.push({Color(), Color()});
}

PainterGtk::~PainterGtk() {
}

void PainterGtk::Save() {
  states_.push(states_.top());
  cairo_save(context_);
}

void PainterGtk::Restore() {
  if (states_.empty())
    return;
  states_.pop();
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

void PainterGtk::ArcTo(const PointF& p1, const PointF& p2, float radius) {
  // Current position.
  double x0, y0;
  cairo_get_current_point(context_, &x0, &y0);
  PointF p0(x0, y0);

  // Draw only a straight line to p1 if any of the points are equal or the
  // radius is zero or the points are collinear (triangle that the points form
  // has area of zero value).
  if (p0 == p1 || p1 == p2 || radius == 0 ||
      AreaOfTriangleFormedByPoints(p0, p1, p2) == 0) {
    cairo_line_to(context_, p1.x(), p1.y());
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
    cairo_line_to(context_, p1.x(), p1.y());
    return;
  }
  if (cos_phi == 1) {
    // Add infinite far away point.
    unsigned int max_length = 65535;
    double factor_max = max_length / p1p0_length;
    PointF ep(p0.x() + factor_max * p1p0.x(), p0.y() + factor_max * p1p0.y());
    cairo_line_to(context_, ep.x(), ep.y());
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

  if (anticlockwise)
    cairo_arc_negative(context_, p.x(), p.y(), radius, sa, ea);
  else
    cairo_arc(context_, p.x(), p.y(), radius, sa, ea);
}

void PainterGtk::Arc(const PointF& point, float radius, float sa, float ea) {
  cairo_arc(context_, point.x(), point.y(), radius, sa, ea);
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

void PainterGtk::Rotate(float angle) {
  cairo_rotate(context_, angle);
}

void PainterGtk::Scale(const Vector2dF& scale) {
  cairo_scale(context_, scale.x(), scale.y());
}

void PainterGtk::SetColor(Color color) {
  states_.top().stroke_color = color;
  states_.top().fill_color = color;
}

void PainterGtk::SetStrokeColor(Color color) {
  states_.top().stroke_color = color;
}

void PainterGtk::SetFillColor(Color color) {
  states_.top().fill_color = color;
}

void PainterGtk::SetLineWidth(float width) {
  cairo_set_line_width(context_, width);
}

void PainterGtk::Stroke() {
  SetSourceColor(true);
  cairo_stroke(context_);
}

void PainterGtk::Fill() {
  SetSourceColor(false);
  cairo_fill(context_);
}

void PainterGtk::StrokeRect(const RectF& rect) {
  cairo_new_path(context_);
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  SetSourceColor(true);
  cairo_stroke(context_);
}

void PainterGtk::FillRect(const RectF& rect) {
  cairo_new_path(context_);
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  SetSourceColor(false);
  cairo_fill(context_);
}

void PainterGtk::DrawImage(Image* image, const RectF& rect) {
}

void PainterGtk::DrawImageFromRect(Image* image, const RectF& src,
                                   const RectF& dest) {
}

TextMetrics PainterGtk::MeasureText(const std::string& text, float width,
                                    const TextAttributes& attributes) {
  PangoLayout* layout = pango_cairo_create_layout(context_);
  pango_layout_set_font_description(layout, attributes.font->GetNative());
  pango_layout_set_text(layout, text.data(), text.length());
  if (width >= 0)
    pango_layout_set_width(layout, width * PANGO_SCALE);
  int bwidth, bheight;
  pango_layout_get_pixel_size(layout, &bwidth, &bheight);
  g_object_unref(layout);
  return { SizeF(bwidth, bheight) };
}

void PainterGtk::DrawTextWithAttributes(
    const std::string& text, const RectF& rect,
    const TextAttributes& attributes) {
  PangoLayout* layout = pango_cairo_create_layout(context_);
  pango_layout_set_font_description(layout, attributes.font->GetNative());
  cairo_save(context_);

  // Text size.
  int width, height;
  pango_layout_set_width(layout, rect.width() * PANGO_SCALE);
  pango_layout_set_text(layout, text.data(), text.length());
  pango_layout_get_pixel_size(layout, &width, &height);

  // Horizontal alignment.
  RectF bounds(rect);
  if (attributes.align == TextAlign::Center)
    bounds.Inset((rect.width() - width) / 2.f, 0.f);
  else if (attributes.align == TextAlign::End)
    bounds.Inset(rect.width() - width, 0.f, 0.f, 0.f);

  // Vertical alignment
  if (attributes.valign == TextAlign::Center)
    bounds.Inset(0.f, (rect.height() - height) / 2);
  else if (attributes.valign == TextAlign::End)
    bounds.Inset(0.f, rect.height() - height, 0.f, 0.f);

  // Apply the color.
  Color color = attributes.color;
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);

  // Draw text.
  cairo_move_to(context_, bounds.x(), bounds.y());
  pango_layout_set_width(layout, bounds.width() * PANGO_SCALE);
  pango_layout_set_height(layout, bounds.height() * PANGO_SCALE);
  pango_cairo_show_layout(context_, layout);

  cairo_restore(context_);
  g_object_unref(layout);
}

void PainterGtk::SetSourceColor(bool stroke) {
  Color color = stroke ? states_.top().stroke_color
                       : states_.top().fill_color;
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);
}

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
