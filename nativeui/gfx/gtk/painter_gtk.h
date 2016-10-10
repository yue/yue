// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PAINTER_GTK_H_
#define NATIVEUI_GFX_GTK_PAINTER_GTK_H_

#include "nativeui/gfx/painter.h"

namespace nu {

class PainterGtk : public Painter {
 public:
  explicit PainterGtk(cairo_t* context);
  ~PainterGtk() override;

  // Painter:
  void Save() override;
  void Restore() override;
  void ClipRect(const RectF& rect,
                CombineMode mode = CombineMode::Replace) override;
  void Translate(const Vector2dF& offset) override;
  void DrawRect(const RectF& rect, Color color) override;
  void FillRect(const RectF& rect, Color color) override;
  void DrawStringWithFlags(const String& text,
                           Font* font,
                           Color color,
                           const RectF& rect,
                           int flags) override;

 private:
  cairo_t* context_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PAINTER_GTK_H_
