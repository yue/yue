// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_APPLICATION_DELEGATE_H_
#define NATIVEUI_MAC_NU_APPLICATION_DELEGATE_H_

#import <Cocoa/Cocoa.h>

namespace nu {
class App;
}

@interface NUApplicationDelegate : NSObject<NSApplicationDelegate> {
 @private
  nu::App* shell_;
}
- (id)initWithShell:(nu::App*)shell;
@end

#endif  // NATIVEUI_MAC_NU_APPLICATION_DELEGATE_H_
