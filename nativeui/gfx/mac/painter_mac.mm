// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/painter_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

PainterMac::PainterMac() {
}

PainterMac::~PainterMac() {
}

void PainterMac::Save() {
  [NSGraphicsContext saveGraphicsState];
}

void PainterMac::Restore() {
  [NSGraphicsContext restoreGraphicsState];
}

void PainterMac::ClipRect(const RectF& rect, CombineMode mode) {
  auto* path = [NSBezierPath bezierPathWithRect:rect.ToCGRect()];
  if (mode == CombineMode::Replace)
    [path setClip];
  else if (mode == CombineMode::Intersect)
    [path addClip];
  else
    LOG(ERROR) << "Cocoa only supports replacing and intersecting clip region";
}

void PainterMac::Translate(const Vector2dF& offset) {
  auto* xform = [NSAffineTransform transform];
  [xform translateXBy:offset.x() yBy:offset.y()];
  [xform concat];
}

void PainterMac::DrawRect(const RectF& rect, Color color) {
  [color.ToNSColor() set];
  NSFrameRect(rect.ToCGRect());
}

void PainterMac::FillRect(const RectF& rect, Color color) {
  [color.ToNSColor() set];
  NSRectFill(rect.ToCGRect());
}

void PainterMac::DrawStringWithFlags(const String& utf8text,
                                     Font font,
                                     Color color,
                                     const RectF& rect,
                                     int flags) {
  NSMutableParagraphStyle* paragraphStyle =
      [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
  NSDictionary* attributes = [NSDictionary
      dictionaryWithObject:paragraphStyle
                    forKey:NSParagraphStyleAttributeName];

  if (flags & TextAlignRight)
    [paragraphStyle setAlignment:NSRightTextAlignment];
  else if (flags & TextAlignCenter)
    [paragraphStyle setAlignment:NSCenterTextAlignment];
  else
    [paragraphStyle setAlignment:NSLeftTextAlignment];

  NSString* text = base::SysUTF8ToNSString(utf8text);
  NSAttributedString* attribute =
      [[[NSAttributedString alloc] initWithString:text
                                       attributes:attributes] autorelease];
  NSRect frame = NSMakeRect(0, (rect.height() - attribute.size.height) / 2,
                            rect.width(), attribute.size.height);
  [text drawInRect:frame withAttributes:attributes];
}

}  // namespace nu
