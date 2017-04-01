// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/painter_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

PainterMac::PainterMac()
    : context_(reinterpret_cast<CGContextRef>(
                   [[NSGraphicsContext currentContext] graphicsPort])) {
}

PainterMac::~PainterMac() {
}

void PainterMac::Save() {
  CGContextSaveGState(context_);
}

void PainterMac::Restore() {
  CGContextRestoreGState(context_);
}

void PainterMac::ClipRect(const RectF& rect, CombineMode mode) {
  if (mode == CombineMode::Replace) {
    CGContextBeginPath(context_);
    CGContextAddRect(context_, rect.ToCGRect());
    CGContextClip(context_);
  } else if (mode == CombineMode::Intersect) {
    CGContextClipToRect(context_, rect.ToCGRect());
  } else {
    LOG(ERROR) << "Cocoa only supports replacing and intersecting clip region";
  }
}

void PainterMac::Translate(const Vector2dF& offset) {
  CGContextTranslateCTM(context_, offset.x(), offset.y());
}

void PainterMac::SetColor(Color color) {
  [color.ToNSColor() set];
}

void PainterMac::SetLineWidth(float width) {
  CGContextSetLineWidth(context_, width);
}

void PainterMac::DrawRect(const RectF& rect) {
  CGContextStrokeRect(context_, rect.ToCGRect());
}

void PainterMac::FillRect(const RectF& rect) {
  CGContextFillRect(context_, rect.ToCGRect());
}

void PainterMac::DrawColoredTextWithFlags(
    const String& str, Font* font, Color color, const RectF& rect, int flags) {
  NSMutableParagraphStyle* paragraphStyle =
      [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
  NSDictionary* attributes = @{
    NSFontAttributeName: font->GetNative(),
    NSParagraphStyleAttributeName: paragraphStyle,
    NSForegroundColorAttributeName: color.ToNSColor(),
  };

  if (flags & TextAlignRight)
    [paragraphStyle setAlignment:NSRightTextAlignment];
  else if (flags & TextAlignCenter)
    [paragraphStyle setAlignment:NSCenterTextAlignment];
  else
    [paragraphStyle setAlignment:NSLeftTextAlignment];

  NSString* text = base::SysUTF8ToNSString(str);
  NSAttributedString* attribute =
      [[[NSAttributedString alloc] initWithString:text
                                       attributes:attributes] autorelease];
  NSRect frame = NSMakeRect(
      rect.x(),
      rect.y() + (rect.height() - attribute.size.height) / 2,
      rect.width(),
       attribute.size.height);
  [text drawInRect:frame withAttributes:attributes];
}

}  // namespace nu
