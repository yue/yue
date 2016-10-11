// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PAINTER_WIN_H_
#define NATIVEUI_GFX_WIN_PAINTER_WIN_H_

#include <stack>

#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

class PainterWin : public Painter {
 public:
  PainterWin(HDC dc, float scale_factor);
  ~PainterWin() override;

  HDC GetHDC();
  void ReleaseHDC(HDC dc);

  // Painter:
  void Save() override;
  void Restore() override;
  void ClipRect(const RectF& rect,
                CombineMode mode = CombineMode::Replace) override;
  void Translate(const Vector2dF& offset) override;
  void SetColor(Color color) override;
  void DrawRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawStringWithFlags(
      const String& text, Font* font, const RectF& rect, int flags) override;

  // The pixel versions.
  void ClipPixelRect(const RectF& rect, CombineMode mode);
  void TranslatePixel(const Vector2dF& offset);
  void DrawPixelRect(const RectF& rect);
  void FillPixelRect(const RectF& rect);
  void DrawPixelStringWithFlags(
      const String& text, Font* font, const RectF& rect, int flags);

  // Helper to get current state.
  Color color() const { return states_.empty() ? color_
                                               : states_.top().color; }
  Vector2dF& origin() { return states_.empty() ? origin_
                                               : states_.top().origin; }

 private:
  // The saved state.
  struct PainterState {
    PainterState(Color color,
                 const Vector2dF& origin,
                 Gdiplus::GraphicsContainer&& container);

    Color color;
    Vector2dF origin;
    Gdiplus::GraphicsContainer container;
  };

  // The stack for all saved states.
  std::stack<PainterState> states_;

  // The root state.
  Color color_;
  Vector2dF origin_;

  float scale_factor_;
  Gdiplus::Graphics graphics_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
