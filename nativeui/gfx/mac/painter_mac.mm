// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/mac/painter_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_cftyperef.h"
#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "nativeui/gfx/canvas.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/image.h"
#include "nativeui/gfx/mac/text_mac.h"

namespace nu {

namespace {

// Create a NSImage from bitmap.
base::scoped_nsobject<NSImage> CreateNSImageFromCanvas(Canvas* canvas) {
  base::ScopedCFTypeRef<CGImageRef> cgimage(
      CGBitmapContextCreateImage(canvas->GetBitmap()));
  base::scoped_nsobject<NSBitmapImageRep> bitmap(
      [[NSBitmapImageRep alloc] initWithCGImage:cgimage]);
  base::scoped_nsobject<NSImage> image(
      [[NSImage alloc] initWithSize:canvas->GetSize().ToCGSize()]);
  [image addRepresentation:bitmap.get()];
  return image;
}

// Switch to the |context| for current scope.
class GraphicsContextScope {
 public:
  explicit GraphicsContextScope(NSGraphicsContext* context)
      : needs_switch_(context != nil) {
    if (needs_switch_) {
      previous_context_ = [NSGraphicsContext currentContext];
      [NSGraphicsContext setCurrentContext:context];
    }
  }

  ~GraphicsContextScope() {
    if (needs_switch_)
      [NSGraphicsContext setCurrentContext:previous_context_];
  }

 private:
  bool needs_switch_;
  NSGraphicsContext* previous_context_;

  DISALLOW_COPY_AND_ASSIGN(GraphicsContextScope);
};

}  // namespace

PainterMac::PainterMac()
    : target_context_(nil),  // no context switching
      context_(reinterpret_cast<CGContextRef>(
                   [[NSGraphicsContext currentContext] graphicsPort])) {
}

PainterMac::PainterMac(NativeBitmap bitmap, float scale_factor)
    : target_context_([[NSGraphicsContext
          graphicsContextWithGraphicsPort:bitmap flipped:YES] retain]),
      context_(bitmap) {
  // There is no way to directly set scale factor for NSGraphicsContext, so just
  // do scaling. The quality of image does not seem to be affected by this.
  Scale(Vector2dF(scale_factor, scale_factor));
}

PainterMac::~PainterMac() {
  [target_context_ release];
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
  SetStrokeColor(color);
  SetFillColor(color);
}

void PainterMac::SetStrokeColor(Color color) {
  CGContextSetRGBStrokeColor(context_, color.r() / 255.f, color.g() / 255.f,
                             color.b() / 255.f, color.a() / 255.f);
}

void PainterMac::SetFillColor(Color color) {
  CGContextSetRGBFillColor(context_, color.r() / 255.f, color.g() / 255.f,
                           color.b() / 255.f, color.a() / 255.f);
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
  GraphicsContextScope scoped(target_context_);
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

  GraphicsContextScope scoped(target_context_);
  [image->GetNative() drawInRect:dest.ToCGRect()
                        fromRect:flipped.ToCGRect()
                       operation:NSCompositeSourceOver
                        fraction:1.0
                  respectFlipped:YES
                           hints:nil];
}

void PainterMac::DrawCanvas(Canvas* canvas, const RectF& rect) {
  base::scoped_nsobject<NSImage> image(CreateNSImageFromCanvas(canvas));
  GraphicsContextScope scoped(target_context_);
  [image drawInRect:rect.ToCGRect()
           fromRect:NSZeroRect
          operation:NSCompositeSourceOver
           fraction:1.0
     respectFlipped:NO
              hints:nil];
}

void PainterMac::DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                                    const RectF& dest) {
  base::scoped_nsobject<NSImage> image(CreateNSImageFromCanvas(canvas));
  GraphicsContextScope scoped(target_context_);
  [image drawInRect:dest.ToCGRect()
           fromRect:src.ToCGRect()
          operation:NSCompositeSourceOver
           fraction:1.0
     respectFlipped:NO
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

  // Options for drawing.
  int options = NSStringDrawingUsesLineFragmentOrigin;
  if (attributes.ellipsis)
    options |= NSStringDrawingTruncatesLastVisibleLine;

  // Vertical alignment.
  RectF bounds(rect);
  float text_height = attributes.wrap ?
      [str boundingRectWithSize:rect.size().ToCGSize()
                        options:options
                     attributes:attrs_dict
                        context:nil].size.height :
      [str sizeWithAttributes:attrs_dict].height;
  // Compute the drawing bounds.
  if (attributes.valign == TextAlign::Start)
    bounds.set_height(text_height);
  else if (attributes.valign == TextAlign::Center)
    bounds.Inset(0.f, (rect.height() - text_height) / 2.f);
  else if (attributes.valign == TextAlign::End)
    bounds.Inset(0.f, rect.height() - text_height, 0.f, 0.f);

  GraphicsContextScope scoped(target_context_);
  [str drawWithRect:bounds.ToCGRect()
            options:options
         attributes:attrs_dict
            context:nil];
}

}  // namespace nu
