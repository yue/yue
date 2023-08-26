// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/mac/legacy_bridging.h"

namespace nu {

// Definitions for the corresponding CF_TO_NS_CAST_DECL macros in
// foundation_util.h.
#define CF_TO_NS_CAST_DEFN(TypeCF, TypeNS) \
\
TypeNS* CFToNSCast(TypeCF##Ref cf_val) { \
  DCHECK(!cf_val || TypeCF##GetTypeID() == CFGetTypeID(cf_val)); \
  TypeNS* ns_val = \
      const_cast<TypeNS*>(reinterpret_cast<const TypeNS*>(cf_val)); \
  return ns_val; \
} \
\
TypeCF##Ref NSToCFCast(TypeNS* ns_val) { \
  TypeCF##Ref cf_val = reinterpret_cast<TypeCF##Ref>(ns_val); \
  DCHECK(!cf_val || TypeCF##GetTypeID() == CFGetTypeID(cf_val)); \
  return cf_val; \
}

#define CF_TO_NS_MUTABLE_CAST_DEFN(name) \
CF_TO_NS_CAST_DEFN(CF##name, NS##name) \
\
NSMutable##name* CFToNSCast(CFMutable##name##Ref cf_val) { \
  DCHECK(!cf_val || CF##name##GetTypeID() == CFGetTypeID(cf_val)); \
  NSMutable##name* ns_val = reinterpret_cast<NSMutable##name*>(cf_val); \
  return ns_val; \
}

CF_TO_NS_MUTABLE_CAST_DEFN(Array)
CF_TO_NS_MUTABLE_CAST_DEFN(AttributedString)
CF_TO_NS_CAST_DEFN(CFCalendar, NSCalendar)
CF_TO_NS_MUTABLE_CAST_DEFN(CharacterSet)
CF_TO_NS_MUTABLE_CAST_DEFN(Data)
CF_TO_NS_CAST_DEFN(CFDate, NSDate)
CF_TO_NS_MUTABLE_CAST_DEFN(Dictionary)
CF_TO_NS_CAST_DEFN(CFError, NSError)
CF_TO_NS_CAST_DEFN(CFLocale, NSLocale)
CF_TO_NS_CAST_DEFN(CFNumber, NSNumber)
CF_TO_NS_CAST_DEFN(CFRunLoopTimer, NSTimer)
CF_TO_NS_CAST_DEFN(CFTimeZone, NSTimeZone)
CF_TO_NS_MUTABLE_CAST_DEFN(Set)
CF_TO_NS_CAST_DEFN(CFReadStream, NSInputStream)
CF_TO_NS_CAST_DEFN(CFWriteStream, NSOutputStream)
CF_TO_NS_MUTABLE_CAST_DEFN(String)
CF_TO_NS_CAST_DEFN(CFURL, NSURL)

#if BUILDFLAG(IS_IOS)
CF_TO_NS_CAST_DEFN(CTFont, UIFont)
#else
CF_TO_NS_CAST_DEFN(CTFont, NSFont)
#endif

#undef CF_TO_NS_CAST_DEFN
#undef CF_TO_NS_MUTABLE_CAST_DEFN

}  // namespace nu
