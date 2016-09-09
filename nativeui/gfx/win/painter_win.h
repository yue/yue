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
  explicit PainterWin(HDC dc);
  ~PainterWin() override;

  HDC GetHDC();
  void ReleaseHDC(HDC dc);

  // Painter:
  void Save() override;
  void Restore() override;
  void ClipRect(const Rect& rect,
                CombineMode mode = CombineMode::Replace) override;
  void Translate(const Vector2d& offset) override;
  void DrawRect(const Rect& rect, Color color) override;
  void DrawRect(const Rect& rect, Pen* pen) override;
  void FillRect(const Rect& rect, Color color) override;
  void DrawStringWithFlags(const String& text,
                           Font font,
                           Color color,
                           const Rect& rect,
                           int flags) override;

  // The translated origin of the painter.
  Vector2d& origin() { return states_.empty() ? origin_
                                              : states_.top().origin; }

 private:
  // The saved state.
  struct PainterState {
    PainterState(const Vector2d& origin,
                 Gdiplus::GraphicsContainer&& container);

    Vector2d origin;
    Gdiplus::GraphicsContainer container;
  };

  // The stack for all saved states.
  std::stack<PainterState> states_;

  // The root origin.
  Vector2d origin_;

  Gdiplus::Graphics graphics_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PAINTER_WIN_H_
