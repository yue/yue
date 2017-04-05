// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/painter_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/mac/text_mac.h"

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

void PainterMac::BeginPath() {
  CGContextBeginPath(context_);
}

void PainterMac::ClosePath() {
  CGContextClosePath(context_);
}

void PainterMac::MoveTo(const PointF& point) {
  CGContextMoveToPoint(context_, point.x(), point.y());
}

void PainterMac::LineTo(const PointF& point) {
  CGContextAddLineToPoint(context_, point.x(), point.y());
}

void PainterMac::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& ep) {
  CGContextAddCurveToPoint(
      context_, cp1.x(), cp1.y(), cp2.x(), cp2.y(), ep.x(), ep.y());
}

void PainterMac::ArcTo(const PointF& cp1, const PointF& cp2, float radius) {
  CGContextAddArcToPoint(context_, cp1.x(), cp1.y(), cp2.x(), cp2.y(), radius);
}

void PainterMac::Arc(const PointF& point, float radius, float sa, float ea) {
  // We are in a flipped coordianate system, so use anti-clockwise.
  CGContextAddArc(context_, point.x(), point.y(), radius, sa, ea, 0);
}

void PainterMac::Rect(const RectF& rect) {
  CGContextAddRect(context_, rect.ToCGRect());
}

void PainterMac::Clip() {
  CGContextClip(context_);
}

void PainterMac::ClipRect(const RectF& rect) {
  CGContextClipToRect(context_, rect.ToCGRect());
}

void PainterMac::Translate(const Vector2dF& offset) {
  CGContextTranslateCTM(context_, offset.x(), offset.y());
}

void PainterMac::Rotate(float angle) {
  CGContextRotateCTM(context_, angle);
}

void PainterMac::Scale(const Vector2dF& scale) {
  CGContextScaleCTM(context_, scale.x(), scale.y());
}

void PainterMac::SetColor(Color color) {
  [color.ToNSColor() set];
}

void PainterMac::SetStrokeColor(Color color) {
  [color.ToNSColor() setStroke];
}

void PainterMac::SetFillColor(Color color) {
  [color.ToNSColor() setFill];
}

void PainterMac::SetLineWidth(float width) {
  CGContextSetLineWidth(context_, width);
}

void PainterMac::Stroke() {
  CGContextStrokePath(context_);
}

void PainterMac::Fill() {
  CGContextFillPath(context_);
}

void PainterMac::StrokeRect(const RectF& rect) {
  CGContextStrokeRect(context_, rect.ToCGRect());
}

void PainterMac::FillRect(const RectF& rect) {
  CGContextFillRect(context_, rect.ToCGRect());
}

SizeF PainterMac::MeasureText(base::StringPiece text, Font* font) {
  return nu::MeasureText(text, font);
}

void PainterMac::DrawColoredTextWithFlags(
    base::StringPiece text, Font* font, Color color, const RectF& rect,
    int flags) {
  NSMutableParagraphStyle* paragraphStyle =
      [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
  NSDictionary* attributes = @{
    NSFontAttributeName: font->GetNative(),
    NSParagraphStyleAttributeName: paragraphStyle,
    NSForegroundColorAttributeName: color.ToNSColor(),
  };

  // Horizontal alignment.
  if (flags & kTextAlignRight)
    [paragraphStyle setAlignment:NSRightTextAlignment];
  else if (flags & kTextAlignCenter)
    [paragraphStyle setAlignment:NSCenterTextAlignment];
  else
    [paragraphStyle setAlignment:NSLeftTextAlignment];

  // Vertical alignment.
  RectF frame(rect);
  if (flags & (kTextAlignVerticalCenter | kTextAlignVerticalBottom)) {
    // Measure text.
    NSString* str = base::SysUTF8ToNSString(text.as_string());
    NSAttributedString* attributed_str =
        [[[NSAttributedString alloc] initWithString:str
                                         attributes:attributes] autorelease];
    CGRect bounds = [attributed_str
        boundingRectWithSize:rect.size().ToCGSize()
                     options:NSStringDrawingUsesLineFragmentOrigin];
    // Adjust the drawing rect.
    if (flags & kTextAlignVerticalCenter)
      frame.Inset(0.f, (rect.height() - bounds.size.height) / 2);
    else
      frame.Inset(0.f, rect.height() - bounds.size.height, 0.f, 0.f);
  }

  [str drawInRect:frame.ToCGRect() withAttributes:attributes];
}

}  // namespace nu
