// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/painter_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/image.h"
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

void PainterMac::DrawImage(Image* image, const RectF& rect) {
  [image->GetNative() drawInRect:rect.ToCGRect()
                        fromRect:NSZeroRect
                       operation:NSCompositeSourceOver
                        fraction:1.0
                  respectFlipped:YES
                           hints:nil];
}

void PainterMac::DrawImageFromRect(Image* image, const RectF& src,
                                   const RectF& dest) {
  // The src rect needs to be manually flipped.
  RectF flipped(src);
  flipped.set_y(image->GetSize().height() - src.height() - src.y());

  [image->GetNative() drawInRect:dest.ToCGRect()
                        fromRect:flipped.ToCGRect()
                       operation:NSCompositeSourceOver
                        fraction:1.0
                  respectFlipped:YES
                           hints:nil];
}

TextMetrics PainterMac::MeasureText(const std::string& text, float width,
                                    const TextAttributes& attributes) {
  return nu::MeasureText(text, width, attributes);
}

void PainterMac::DrawText(const std::string& text, const RectF& rect,
                          const TextAttributes& attributes) {
  NSString* str = base::SysUTF8ToNSString(text);

  // Horizontal alignment.
  base::scoped_nsobject<NSMutableParagraphStyle> paragraph(
      [[NSParagraphStyle defaultParagraphStyle] mutableCopy]);
  switch (attributes.align) {
    case TextAlign::Start:
      [paragraph setAlignment:NSLeftTextAlignment];
      break;
    case TextAlign::Center:
      [paragraph setAlignment:NSCenterTextAlignment];
      break;
    case TextAlign::End:
      [paragraph setAlignment:NSRightTextAlignment];
      break;
  }

  // Attributes passed to Cocoa.
  NSDictionary* attrs_dict = @{
    NSFontAttributeName: attributes.font->GetNative(),
    NSParagraphStyleAttributeName: paragraph.get(),
    NSForegroundColorAttributeName: attributes.color.ToNSColor(),
  };

  // Vertical alignment.
  RectF bounds(rect);
  if (attributes.valign != TextAlign::Start) {
    // Measure text.
    base::scoped_nsobject<NSAttributedString> attributed_str(
        [[NSAttributedString alloc] initWithString:str attributes:attrs_dict]);
    CGRect cg_bounds = [attributed_str
        boundingRectWithSize:rect.size().ToCGSize()
                     options:NSStringDrawingUsesLineFragmentOrigin];
    // Compute the drawing bounds.
    if (attributes.valign == TextAlign::Center)
      bounds.Inset(0.f, (rect.height() - cg_bounds.size.height) / 2);
    else if (attributes.valign == TextAlign::End)
      bounds.Inset(0.f, rect.height() - cg_bounds.size.height, 0.f, 0.f);
  }

  [str drawInRect:bounds.ToCGRect() withAttributes:attrs_dict];
}

}  // namespace nu
