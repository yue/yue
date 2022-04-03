// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

#import <Cocoa/Cocoa.h>

#include "base/notreached.h"
#include "base/mac/foundation_util.h"
#include "base/mac/scoped_nsobject.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"

namespace nu {

namespace {

const int kPollingTimeout = 500;

const char kMarkupPrefix[] = "<meta charset='utf-8'>";

// If there is RTF data on the pasteboard, returns an HTML version of it.
// Otherwise returns nil.
NSString* GetHTMLFromRTFOnPasteboard(NSPasteboard* pboard) {
  NSData* rtfData = [pboard dataForType:NSRTFPboardType];
  if (!rtfData)
    return nil;
  auto* rtf = [[[NSAttributedString alloc] initWithRTF:rtfData
                                    documentAttributes:nil] autorelease];
  NSDictionary* attributes = @{
     NSDocumentTypeDocumentAttribute: NSHTMLTextDocumentType
  };
  NSData* htmlData = [rtf dataFromRange:NSMakeRange(0, [rtf length])
                     documentAttributes:attributes
                                  error:nil];
  // According to the docs, NSHTMLTextDocumentType is UTF8.
  return [[[NSString alloc] initWithData:htmlData
                                encoding:NSUTF8StringEncoding] autorelease];
}

}  // namespace

Clipboard::Clipboard(NativeClipboard clipboard)
    : type_(Type::Drag), clipboard_(clipboard), weak_factory_(this) {}

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

bool Clipboard::IsDataAvailable(Data::Type type) const {
  NSArray* types = clipboard_.types;
  switch (type) {
    case Data::Type::Text:
      return [types containsObject:NSPasteboardTypeString];
    case Data::Type::HTML:
      return [types containsObject:NSHTMLPboardType] ||
             [types containsObject:NSRTFPboardType];
    case Data::Type::Image:
      return [types containsObject:NSPasteboardTypeTIFF];
    case Data::Type::FilePaths:
      return [types containsObject:NSFilenamesPboardType];
    default:
      NOTREACHED() << "Can not get clipboard data without type";
      return false;
  }
}

Clipboard::Data Clipboard::GetData(Data::Type type) const {
  switch (type) {
    case Data::Type::Text: {
      NSString* str = [clipboard_ stringForType:NSPasteboardTypeString];
      return str ? Data(Data::Type::Text, base::SysNSStringToUTF8(str))
                 : Data();
    }
    case Data::Type::HTML: {
      NSArray* supportedTypes = @[NSHTMLPboardType, NSRTFPboardType];
      NSString* bestType = [clipboard_ availableTypeFromArray:supportedTypes];
      if (!bestType)
        return Data();
      NSString* contents;
      if ([bestType isEqualToString:NSRTFPboardType])
        contents = GetHTMLFromRTFOnPasteboard(clipboard_);
      else
        contents = [clipboard_ stringForType:bestType];
      // The HTML may have meta prefix in it, remove it for consistent result.
      std::string html = base::SysNSStringToUTF8(contents);
      if (base::StartsWith(html, kMarkupPrefix, base::CompareCase::SENSITIVE))
        html = html.substr(std::size(kMarkupPrefix) - 1,
                           html.length() - (std::size(kMarkupPrefix) - 1));
      return Data(Data::Type::HTML, std::move(html));
    }
    case Data::Type::Image: {
      // If the pasteboard's image data is not to its liking, the guts of
      // NSImage may throw, and that exception will leak. Prevent a crash in
      // that case; a blank image is better.
      base::scoped_nsobject<NSImage> image;
      @try {
        image.reset([[NSImage alloc] initWithPasteboard:clipboard_]);
      } @catch (id exception) {
      }
      return image ? Data(new Image(image.release())) : Data();
    }
    case Data::Type::FilePaths: {
      NSArray* paths = [clipboard_ propertyListForType:NSFilenamesPboardType];
      if (!paths)
        return Data();
      std::vector<base::FilePath> result;
      result.reserve([paths count]);
      for (NSString* path in paths)
        result.push_back(base::mac::NSStringToFilePath(path));
      return Data(std::move(result));
    }
    default:
      NOTREACHED() << "Can not get clipboard data without type";
      return Data();
  }
}

void Clipboard::SetData(std::vector<Data> objects) {
  [clipboard_ declareTypes:@[] owner:nil];

  for (const auto& data : objects) {
    switch (data.type()) {
      case Data::Type::Text:
        [clipboard_ addTypes:@[NSPasteboardTypeString] owner:nil];
        [clipboard_ setString:base::SysUTF8ToNSString(data.str())
                      forType:NSPasteboardTypeString];
        break;
      case Data::Type::HTML: {
        [clipboard_ addTypes:@[NSHTMLPboardType] owner:nil];
        // We need to mark it as utf-8. (see crbug.com/11957)
        std::string html = kMarkupPrefix + data.str();
        [clipboard_ setString:base::SysUTF8ToNSString(html)
                      forType:NSHTMLPboardType];
        break;
      }
      case Data::Type::Image:
        [clipboard_ writeObjects:@[data.image()->GetNative()]];
        break;
      case Data::Type::FilePaths: {
        NSMutableArray* filePaths = [NSMutableArray array];
        for (const auto& path : data.file_paths())
          [filePaths addObject:base::SysUTF8ToNSString(path.value())];
        [clipboard_ addTypes:@[NSFilenamesPboardType] owner:nil];
        [clipboard_ setPropertyList:filePaths forType:NSFilenamesPboardType];
        break;
      }
      default:
        NOTREACHED() << "Can not set clipboard data without type";
    }
  }
}

void Clipboard::PlatformStartWatching() {
  DCHECK_EQ(timer_, 0u);
  change_count_ = [clipboard_ changeCount];
  timer_ = MessageLoop::SetTimeout(kPollingTimeout,
                                   std::bind(&Clipboard::OnTimer, this));
}

void Clipboard::PlatformStopWatching() {
  DCHECK_GT(timer_, 0u);
  MessageLoop::ClearTimeout(timer_);
  timer_ = 0;
}

void Clipboard::OnTimer() {
  timer_ = MessageLoop::SetTimeout(kPollingTimeout,
                                   std::bind(&Clipboard::OnTimer, this));
  if (change_count_ != [clipboard_ changeCount]) {
    change_count_ = [clipboard_ changeCount];
    on_change.Emit(this);
  }
}

}  // namespace nu
