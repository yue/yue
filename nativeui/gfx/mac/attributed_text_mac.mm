// Copyright 2020 Cheng Zhao. All rights reserved.
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

AttributedText::AttributedText(const std::string& text, TextAttributes att)
    : text_([[NSMutableAttributedString alloc]
                initWithString:base::SysUTF8ToNSString(text)]),
      format_(att.ToTextFormat()) {
  [text_ beginEditing];
  SetFont(att.font.get());
  SetColor(att.color);
  [text_ endEditing];
}

AttributedText::~AttributedText() {
  [text_ release];
}

void AttributedText::PlatformUpdateFormat() {
}

void AttributedText::PlatformSetFontFor(scoped_refptr<Font> font,
                                        int start, int end) {
  [text_ beginEditing];
  if (start == 0 && end == -1) {
    [text_ removeAttribute:NSFontAttributeName
                     range:NSMakeRange(0, [text_ length])];
  }
  [text_ addAttribute:NSFontAttributeName
                value:font->GetNative()
                range:NSMakeRange(start, IndexToLength(text_, start, end))];
  [text_ endEditing];
}

void AttributedText::PlatformSetColorFor(Color color, int start, int end) {
  [text_ beginEditing];
  if (start == 0 && end == -1) {
    [text_ removeAttribute:NSForegroundColorAttributeName
                     range:NSMakeRange(0, [text_ length])];
  }
  [text_ addAttribute:NSForegroundColorAttributeName
                value:color.ToNSColor()
                range:NSMakeRange(start, IndexToLength(text_, start, end))];
  [text_ endEditing];
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

void AttributedText::SetText(const std::string& text) {
  [[text_ mutableString] setString:base::SysUTF8ToNSString(text)];
}

std::string AttributedText::GetText() const {
  return base::SysNSStringToUTF8([text_ string]);
}

}  // namespace nu
