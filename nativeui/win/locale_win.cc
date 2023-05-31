// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/locale.h"

#include <windows.h>

#include "base/strings/sys_string_conversions.h"
#include "base/win/i18n.h"

namespace nu {

std::string Locale::GetCurrentIdentifier() {
  wchar_t locale[LOCALE_NAME_MAX_LENGTH] = {0};
  if (::GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH) == 0)
    return "en-US";
  return base::SysWideToUTF8(locale);
}

std::vector<std::string> Locale::GetPreferredLanguages() {
  std::vector<std::wstring> languages;
  base::win::i18n::GetThreadPreferredUILanguageList(&languages);
  std::vector<std::string> result;
  for (const std::wstring& language : languages)
    result.push_back(base::SysWideToUTF8(language));
  return result;
}

}  // namespace nu
