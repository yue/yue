// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_MAC_PAINTER_MAC_H_
#define NATIVEUI_GFX_MAC_PAINTER_MAC_H_

#include "nativeui/gfx/painter.h"

namespace nu {

class PainterMac : public Painter {
 public:
  PainterMac();
  ~PainterMac() override;

  // Painter:
  void Save() override;
  void Restore() override;
  void ClipRect(const RectF& rect,
                CombineMode mode = CombineMode::Replace) override;
  void Translate(const Vector2dF& offset) override;
  void SetColor(Color color) override;
  void DrawRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawTextWithFlags(
      const String& text, Font* font, const RectF& rect, int flags) override;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_MAC_PAINTER_MAC_H_
