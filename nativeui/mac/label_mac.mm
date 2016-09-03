// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/text.h"

@interface LabelView : NSView {
 @private
  NSString* text_;
}
@property(nonatomic, copy) NSString* text;
@end

@implementation LabelView

@synthesize text = text_;

- (void)drawRect:(NSRect)dirtyRect {
  NSMutableParagraphStyle* paragraphStyle =
      [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
  [paragraphStyle setAlignment:NSCenterTextAlignment];
  NSDictionary* attributes = [NSDictionary
      dictionaryWithObject:paragraphStyle
                    forKey:NSParagraphStyleAttributeName];
  NSAttributedString* text =
      [[[NSAttributedString alloc] initWithString:text_
                                       attributes:attributes] autorelease];
  NSRect frame = NSMakeRect(0, (self.frame.size.height - text.size.height) / 2,
                            self.frame.size.width, text.size.height);
  [text_ drawInRect:frame withAttributes:attributes];
}

@end

namespace nu {

namespace {

Size GetPreferredSizeFroText(const std::string& text) {
  Size size = ToCeiledSize(MeasureText(Font(), text));
  size.Enlarge(1, 1);  // leave space for border
  return size;
}

}  // namespace

Label::Label(const std::string& text) {
  TakeOverView([[LabelView alloc] init]);
  SetText(text);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  LabelView* label = static_cast<LabelView*>(view());
  label.text = base::SysUTF8ToNSString(text);
  label.needsDisplay = YES;
  SetPreferredSize(GetPreferredSizeFroText(text));
}

std::string Label::GetText() {
  return base::SysNSStringToUTF8(static_cast<LabelView*>(view()).text);
}

}  // namespace nu
