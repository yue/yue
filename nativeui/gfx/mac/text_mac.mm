// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/text_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"

namespace nu {

TextMetrics MeasureText(const std::string& text, float width,
                        const TextAttributes& attributes) {
  NSDictionary* attrs_dict = @{
     NSFontAttributeName: attributes.font->GetNative(),
  };
  base::scoped_nsobject<NSAttributedString> attributed_str(
      [[NSAttributedString alloc] initWithString:base::SysUTF8ToNSString(text)
                                      attributes:attrs_dict]);
  CGRect bounds = [attributed_str
      boundingRectWithSize:CGSizeMake(width, -1)
                   options:NSStringDrawingUsesLineFragmentOrigin];
  return { SizeF(bounds.size) };
}

}  // namespace nu
