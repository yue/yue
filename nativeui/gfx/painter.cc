// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/painter.h"

namespace nu {

Painter::Painter() : weak_factory_(this) {}

Painter::~Painter() {}

void Painter::DrawText(base::StringPiece text, Font* font, const RectF& rect) {
  // TODO(zcbenz): Support RTL in future.
  DrawTextWithFlags(text, font, rect, TextAlignLeft);
}

void Painter::DrawTextWithFlags(base::StringPiece text, Font* font,
                                const RectF& rect, int flags) {
  DrawColoredTextWithFlags(
      text, font, GetSystemColor(SystemColor::Text), rect, flags);
}

void Painter::DrawColoredText(base::StringPiece text, Font* font, Color color,
                              const RectF& rect) {
  // TODO(zcbenz): Support RTL in future.
  DrawColoredTextWithFlags(text, font, color, rect, TextAlignLeft);
}

}  // namespace nu
