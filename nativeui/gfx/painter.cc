// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/painter.h"

#include "nativeui/gfx/attributed_text.h"

namespace nu {

Painter::Painter() : weak_factory_(this) {}

Painter::~Painter() {}

void Painter::DrawText(const std::string& str, const RectF& rect,
                       const TextAttributes& attributes) {
  DrawAttributedText(new AttributedText(str, attributes), rect);
}

}  // namespace nu
