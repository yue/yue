// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/text.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

SizeF MeasureText(Font* font, base::StringPiece text) {
  NSMutableParagraphStyle* paragraphStyle =
      [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
  [paragraphStyle setAlignment:NSCenterTextAlignment];
  NSDictionary* attrs = @{
    NSParagraphStyleAttributeName : paragraphStyle,
    NSFontAttributeName : font->GetNative()
  };
  NSAttributedString* attribute =
      [[[NSAttributedString alloc] initWithString:base::SysUTF8ToNSString(text)
                                       attributes:attrs] autorelease];
  return SizeF(attribute.size.width, attribute.size.height);
}

}  // namespace nu
