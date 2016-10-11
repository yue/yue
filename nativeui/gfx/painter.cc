// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/painter.h"

namespace nu {

Painter::Painter() : weak_factory_(this) {}

Painter::~Painter() {}

void Painter::DrawText(const String& text, Font* font, const RectF& rect) {
  // TODO(zcbenz): Support RTL in future.
  DrawTextWithFlags(text, font, rect, TextAlignLeft);
}

}  // namespace nu
