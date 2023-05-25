// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/locale.h"

#include "base/notreached.h"

namespace nu {

#if !defined(OS_MAC) && !defined(OS_WIN)
std::string Locale::GetCurrentIdentifier() {
  auto languages = GetPreferredLanguages();
  if (languages.empty())  // can happen in some CI environment
    return "en-US";
  return languages[0];
}
#endif

}  // namespace nu
