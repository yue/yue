// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/locale.h"

#include <gtk/gtk.h>

#include <algorithm>

#include "base/strings/string_piece.h"

namespace nu {

namespace {

std::string NormalizeLocaleString(base::StringPiece locale) {
  // Remove ".UTF-8" suffix.
  size_t dot = locale.find('.');
  if (dot != base::StringPiece::npos)
    locale = locale.substr(0, dot);
  // Convert "_" to "-".
  std::string result(locale.data(), locale.size());
  size_t underscore = result.find('_');
  if (underscore != base::StringPiece::npos)
    result[underscore] = '-';
  return result;
}

}  // namespace

std::vector<std::string> Locale::GetPreferredLanguages() {
  std::vector<std::string> result;
  const char* const* languages = g_get_language_names();
  for (; *languages; ++languages) {
    if (**languages == 'C')  // remove "C" locale
      continue;
    result.push_back(NormalizeLocaleString(*languages));
  }
  // Remove duplicate items which happen after removing ".UTF-8" suffix.
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

}  // namespace nu
