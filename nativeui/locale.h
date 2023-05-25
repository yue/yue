// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LOCALE_H_
#define NATIVEUI_LOCALE_H_

#include <string>
#include <vector>

#include "nativeui/nativeui_export.h"

namespace nu {

class NATIVEUI_EXPORT Locale {
 public:
  static std::string GetCurrentIdentifier();
  static std::vector<std::string> GetPreferredLanguages();
};

}  // namespace nu

#endif  // NATIVEUI_LOCALE_H_
