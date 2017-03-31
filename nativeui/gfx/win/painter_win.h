// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PAINTER_WIN_H_
#define NATIVEUI_GFX_WIN_PAINTER_WIN_H_

#include <stack>

#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/gdiplus.h"
#include "nativeui/gfx/win/native_theme.h"

namespace nu {

class PainterWin : public Painter {
 public:
  PainterWin(HDC hdc, float scale_factor);
  ~PainterWin() override;

  // Receive the HDC that can be painted on.
  // TODO(zcbenz): Remove all usages of this.
  HDC GetHDC();
  void ReleaseHDC(HDC dc);

  // Draw a control.
  void DrawNativeTheme(NativeTheme::Part part,
                       ControlState state,
                       const Rect& rect,
                       const NativeTheme::ExtraParams& extra);

  // Painter:
  void Save() override;
  void Restore() override;
  void ClipRect(const RectF& rect,
                CombineMode mode = CombineMode::Replace) override;
  void Translate(const Vector2dF& offset) override;
  void SetColor(Color color) override;
  void SetLineWidth(float width) override;
  void DrawRect(const RectF& rect) override;
  void FillRect(const RectF& rect) override;
  void DrawColoredTextWithFlags(
      const String& text, Font* font, Color color, const RectF& rect,
      int flags) override;

  // The pixel versions.
  void ClipPixelRect(const RectF& rect, CombineMode mode);
  void TranslatePixel(const Vector2dF& offset);
  void SetPixelLineWidth(float width);
  void DrawPixelRect(const RectF& rect);
  void FillPixelRect(const RectF& rect);
  void DrawColoredTextPixelWithFlags(
      const String& text, Font* font, Color color, const RectF& rect,
      int flags);

  // Helper to get current state.
  Color& color() { return states_.empty() ? color_
                                          : states_.top().color; }
  float& line_width() { return states_.empty() ? line_width_
                                               : states_.top().line_width; }
  Vector2dF& origin() { return states_.empty() ? origin_
                                               : states_.top().origin; }

  // The internal HDC handle.
  HDC hdc() const { return hdc_; }

 private:
  // The saved state.
  struct PainterState {
    PainterState(Color color,
                 const Vector2dF& origin,
                 Gdiplus::GraphicsContainer&& container);

    Color color;
    float line_width = 1.f;
    Vector2dF origin;
    Gdiplus::GraphicsContainer container;
  };

  // The stack for all saved states.
  std::stack<PainterState> states_;

  // The root state.
  Color color_;
  float line_width_ = 1.f;
  Vector2dF origin_;

  // Weak ref to the original HDC, should only be used internally.
  HDC hdc_;

  float scale_factor_;
  Gdiplus::Graphics graphics_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
