// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

#import <Cocoa/Cocoa.h>

#include "base/strings/sys_string_conversions.h"

namespace nu {

NativeClipboard Clipboard::PlatformCreate(Type type) {
  switch (type) {
    case Type::CopyPaste:
      return [NSPasteboard generalPasteboard];
    case Type::Drag:
      return [NSPasteboard pasteboardWithName:NSDragPboard];
    case Type::Find:
      return [NSPasteboard pasteboardWithName:NSFindPboard];
    case Type::Font:
      return [NSPasteboard pasteboardWithName:NSFontPboard];
    default:
      NOTREACHED() << "Type::Count is not a valid clipboard type";
  }
  return [NSPasteboard generalPasteboard];
}

void Clipboard::PlatformDestroy() {
}

void Clipboard::Clear() {
  [clipboard_ clearContents];
}

void Clipboard::SetText(const std::string& text) {
  [clipboard_ declareTypes:@[NSPasteboardTypeString] owner:nil];
  [clipboard_ setString:base::SysUTF8ToNSString(text)
                forType:NSPasteboardTypeString];
}

std::string Clipboard::GetText() const {
  return base::SysNSStringToUTF8(
      [clipboard_ stringForType:NSPasteboardTypeString]);
}

}  // namespace nu
