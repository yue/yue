// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#import <Foundation/Foundation.h>

#include "nativeui/locale.h"

#include "base/strings/sys_string_conversions.h"

namespace nu {

std::string Locale::GetCurrentIdentifier() {
  NSLocale* locale = [NSLocale autoupdatingCurrentLocale];
  return base::SysNSStringToUTF8([locale languageCode]) + "-" +
         base::SysNSStringToUTF8([locale countryCode]);
}

std::vector<std::string> Locale::GetPreferredLanguages() {
  std::vector<std::string> result;
  NSArray<NSString*>* languages = [NSLocale preferredLanguages];
  for (NSString* language in languages)
    result.push_back(base::SysNSStringToUTF8(language));
  return result;
}

}  // namespace nu
