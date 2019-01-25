// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/cursor.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"

@interface NSCursor(Undocumented)
+ (NSCursor*)busyButClickableCursor;
@end

namespace nu {

namespace {

NSCursor* LoadFromHIServices(NSString* name) {
  NSString* cursorPath =
      [@"/System/Library/Frameworks/ApplicationServices.framework"
        "/Versions/A/Frameworks/HIServices.framework"
        "/Versions/A/Resources/cursors" stringByAppendingPathComponent:name];
  NSString* imagePath =
      [cursorPath stringByAppendingPathComponent:@"cursor.pdf"];
  base::scoped_nsobject<NSImage> image(
      [[NSImage alloc] initByReferencingFile:imagePath]);
  NSString* plistPath =
      [cursorPath stringByAppendingPathComponent:@"info.plist"];
  NSDictionary* info = [NSDictionary dictionaryWithContentsOfFile:plistPath];
  NSPoint hotSpot = NSMakePoint(
      [[info valueForKey:@"hotx"] doubleValue],
      [[info valueForKey:@"hoty"] doubleValue]);
  return [[NSCursor alloc] initWithImage:image hotSpot:hotSpot];
}

}  // namespace

Cursor::Cursor(Type type) {
  switch (type) {
    case Type::Default:
      cursor_ = [[NSCursor arrowCursor] retain];
      break;
    case Type::Hand:
      cursor_ = [[NSCursor pointingHandCursor] retain];
      break;
    case Type::Crosshair:
      cursor_ = [[NSCursor crosshairCursor] retain];
      break;
    case Type::Progress:
      cursor_ = [[NSCursor busyButClickableCursor] retain];
      break;
    case Type::Text:
      cursor_ = [[NSCursor IBeamCursor] retain];
      break;
    case Type::NotAllowed:
      cursor_ = [[NSCursor operationNotAllowedCursor] retain];
      break;
    case Type::Help:
      cursor_ = LoadFromHIServices(@"help");
      break;
    case Type::Move:
      cursor_ = LoadFromHIServices(@"move");
      break;
    case Type::ResizeEW:
      cursor_ = LoadFromHIServices(@"resizeeastwest");
      break;
    case Type::ResizeNS:
      cursor_ = LoadFromHIServices(@"resizenorthsouth");
      break;
    case Type::ResizeNESW:
      cursor_ = LoadFromHIServices(@"resizenortheastsouthwest");
      break;
    case Type::ResizeNWSE:
      cursor_ = LoadFromHIServices(@"resizenorthwestsoutheast");
      break;
  }
}

Cursor::~Cursor() {
  [cursor_ release];
}

}  // namespace nu
