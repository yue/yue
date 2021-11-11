// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_BROWSER_NU_CUSTOM_PROTOCOL_H_
#define NATIVEUI_MAC_BROWSER_NU_CUSTOM_PROTOCOL_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/browser.h"

@interface NUCustomProtocol : NSURLProtocol {
 @private
  nu::ProtocolJob* protocol_job_;
}
+ (bool)registerProtocol:(NSString*)scheme
             withHandler:(nu::Browser::ProtocolHandler)handler;
+ (bool)unregisterProtocol:(NSString*)scheme;
@end

#endif  // NATIVEUI_MAC_BROWSER_NU_CUSTOM_PROTOCOL_H_
