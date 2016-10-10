// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
#define NATIVEUI_GFX_GTK_PAINTER_GTK_CC_

#include "nativeui/gfx/gtk/painter_gtk.h"

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

void PainterGtk::ClipRect(const RectF& rect, CombineMode mode) {
  if (mode == CombineMode::Union || mode == CombineMode::Exclude) {
    LOG(ERROR) << "Cairo only supports replacing and intersecting clip region";
    return;
  } else if (mode == CombineMode::Replace) {
    cairo_reset_clip(context_);
  }
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_clip(context_);
}

void PainterGtk::Translate(const Vector2dF& offset) {
  cairo_translate(context_, offset.x(), offset.y());
}

void PainterGtk::DrawRect(const RectF& rect, Color color) {
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);
  cairo_stroke(context_);
}

void PainterGtk::FillRect(const RectF& rect, Color color) {
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);
  cairo_fill(context_);
}

void PainterGtk::DrawStringWithFlags(const String& text,
                                     Font* font,
                                     Color color,
                                     const RectF& rect,
                                     int flags) {
}

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
