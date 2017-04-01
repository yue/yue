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

  // Draw a control.
  void DrawNativeTheme(NativeTheme::Part part,
                       ControlState state,
                       const Rect& rect,
                       const NativeTheme::ExtraParams& extra);

  // Draw the focus rect.
  void DrawFocusRect(const Rect& rect);

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
  void DrawPixelRect(const RectF& rect);
  void FillPixelRect(const RectF& rect);
  void DrawColoredTextPixelWithFlags(
      const String& text, Font* font, Color color, const RectF& rect,
      int flags);

 private:
  // The saved state.
  struct PainterState {
    PainterState(float line_width, Color color)
        : line_width(line_width), color(color) {}
    float line_width;
    Color color;
    int state = 0;
  };

  // Return the top state.
  PainterState& top() { return states_.top(); }

  // The stack for all saved states.
  std::stack<PainterState> states_;

  // Weak ref to the original HDC, should only be used internally.
  HDC hdc_;

  float scale_factor_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
