// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/attributed_text.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/rect_f.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/gfx/text.h"

namespace nu {

namespace {

inline int IndexToLength(NSAttributedString* str, int start, int end) {
  return end > -1 ? end - start : [str length] - start;
}

}  // namespace

AttributedText::AttributedText(const std::string& text,
                               const TextFormat& format)
    : text_([[NSMutableAttributedString alloc]
                initWithString:base::SysUTF8ToNSString(text)]),
      format_(format) {}

AttributedText::~AttributedText() {
  [text_ release];
}

void AttributedText::PlatformSetFontFor(Font* font, int start, int end) {
  [text_ addAttribute:NSFontAttributeName
                value:font->GetNative()
                range:NSMakeRange(start, IndexToLength(text_, start, end))];
}

void AttributedText::PlatformSetColorFor(Color color, int start, int end) {
  [text_ addAttribute:NSForegroundColorAttributeName
                value:color.ToNSColor()
                range:NSMakeRange(start, IndexToLength(text_, start, end))];
}

RectF AttributedText::GetBoundsFor(const SizeF& size) const {
  int draw_options = 0;
  if (format_.wrap)
    draw_options |= NSStringDrawingUsesLineFragmentOrigin;
  if (format_.ellipsis)
    draw_options |= NSStringDrawingTruncatesLastVisibleLine;
  if (format_.wrap || format_.ellipsis) {
    // The height returned by boundingRectWithSize can not be larger than passed
    // size by default, which is not really expected behavior.
    NSSize bsize = format_.ellipsis ? size.ToCGSize()
                                    : NSMakeSize(size.width(), FLT_MAX);
    return RectF([text_ boundingRectWithSize:bsize
                                     options:draw_options]);
  } else {
    // boundingRectWithSize does not work well for single line text.
    return RectF(SizeF([text_ size]));
  }
}

std::string AttributedText::GetText() const {
  return base::SysNSStringToUTF8([text_ string]);
}

}  // namespace nu
