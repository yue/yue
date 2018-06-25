// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_APPLICATION_DELEGATE_H_
#define NATIVEUI_MAC_NU_APPLICATION_DELEGATE_H_

#import <Cocoa/Cocoa.h>

namespace nu {
class Lifetime;
}

@interface NUApplicationDelegate : NSObject<NSApplicationDelegate> {
 @private
  nu::Lifetime* shell_;
 @private
  NSMutableArray *fileURLPaths;
}
- (id)initWithShell:(nu::Lifetime*)shell;
@end

#endif  // NATIVEUI_MAC_NU_APPLICATION_DELEGATE_H_
