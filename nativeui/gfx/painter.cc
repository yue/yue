// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/painter.h"

namespace nu {

TextAttributes::TextAttributes()
    : font(nu::State::GetCurrent()->GetDefaultFont()),
      color(GetSystemColor(SystemColor::Text)),
      align(TextAlign::Start),
      valign(TextAlign::Start) {
}

Painter::Painter() : weak_factory_(this) {}

Painter::~Painter() {}

void Painter::DrawText(const std::string& text, const RectF& rect) {
  // TODO(zcbenz): Support RTL in future.
  DrawTextWithAttributes(text, rect, TextAttributes());
}

}  // namespace nu
