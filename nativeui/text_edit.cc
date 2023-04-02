// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include "nativeui/gfx/attributed_text.h"

namespace nu {

// static
const char TextEdit::kClassName[] = "TextEdit";

const char* TextEdit::GetClassName() const {
  return kClassName;
}

#if defined(OS_MAC) || defined(OS_LINUX)
RectF TextEdit::GetTextBounds() const {
  scoped_refptr<AttributedText> attributed_text =
      new AttributedText(GetText(), TextFormat());
  if (font())
    attributed_text->SetFont(font());
  return attributed_text->GetBoundsFor(SizeF(GetBounds().width(), FLT_MAX));
}
#endif

}  // namespace nu
