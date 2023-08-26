// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_MAC_LEGACY_BRIDGING_H_
#define NATIVEUI_MAC_LEGACY_BRIDGING_H_

// This header provides Chromium removed helpers CFToNSCast and NSToCFCast, they
// are only meant for smooth migration to enable ARC in nativeui.

#include "base/apple/foundation_util.h"

// These casting functions cannot be implemented in a way that will work with
// ARC. Use the casting functions in base/apple/bridging.h instead.
#if !defined(__has_feature) || !__has_feature(objc_arc)

#if defined(__OBJC__)

// Convert toll-free bridged CFTypes to NSTypes and vice-versa. This does not
// autorelease |cf_val|. This is useful for the case where there is a CFType in
// a call that expects an NSType and the compiler is complaining about const
// casting problems.
// The calls are used like this:
// NSString *foo = CFToNSCast(CFSTR("Hello"));
// CFStringRef foo2 = NSToCFCast(@"Hello");
// The macro magic below is to enforce safe casting. It could possibly have
// been done using template function specialization, but template function
// specialization doesn't always work intuitively,
// (http://www.gotw.ca/publications/mill17.htm) so the trusty combination
// of macros and function overloading is used instead.

#define CF_TO_NS_CAST_DECL(TypeCF, TypeNS) \
  namespace nu {                           \
  TypeNS* CFToNSCast(TypeCF##Ref cf_val);  \
  TypeCF##Ref NSToCFCast(TypeNS* ns_val);  \
  }

#define CF_TO_NS_MUTABLE_CAST_DECL(name)                    \
  CF_TO_NS_CAST_DECL(CF##name, NS##name)                    \
                                                            \
  namespace nu {                                            \
  NSMutable##name* CFToNSCast(CFMutable##name##Ref cf_val); \
  CFMutable##name##Ref NSToCFCast(NSMutable##name* ns_val); \
  }

// List of toll-free bridged types taken from:
// http://www.cocoadev.com/index.pl?TollFreeBridged
CF_TO_NS_MUTABLE_CAST_DECL(Array)
CF_TO_NS_MUTABLE_CAST_DECL(AttributedString)
CF_TO_NS_CAST_DECL(CFCalendar, NSCalendar)
CF_TO_NS_MUTABLE_CAST_DECL(CharacterSet)
CF_TO_NS_MUTABLE_CAST_DECL(Data)
CF_TO_NS_CAST_DECL(CFDate, NSDate)
CF_TO_NS_MUTABLE_CAST_DECL(Dictionary)
CF_TO_NS_CAST_DECL(CFError, NSError)
CF_TO_NS_CAST_DECL(CFLocale, NSLocale)
CF_TO_NS_CAST_DECL(CFNumber, NSNumber)
CF_TO_NS_CAST_DECL(CFRunLoopTimer, NSTimer)
CF_TO_NS_CAST_DECL(CFTimeZone, NSTimeZone)
CF_TO_NS_MUTABLE_CAST_DECL(Set)
CF_TO_NS_CAST_DECL(CFReadStream, NSInputStream)
CF_TO_NS_CAST_DECL(CFWriteStream, NSOutputStream)
CF_TO_NS_MUTABLE_CAST_DECL(String)
CF_TO_NS_CAST_DECL(CFURL, NSURL)

#if BUILDFLAG(IS_IOS)
CF_TO_NS_CAST_DECL(CTFont, UIFont)
#else
CF_TO_NS_CAST_DECL(CTFont, NSFont)
#endif

#undef CF_TO_NS_CAST_DECL
#undef CF_TO_NS_MUTABLE_CAST_DECL
#undef OBJC_CPP_CLASS_DECL

#endif  // __OBJC__

#endif  // !defined(__has_feature) || !__has_feature(objc_arc)

#endif  // NATIVEUI_MAC_LEGACY_BRIDGING_H_
