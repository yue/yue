// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_GTK_UTIL_FONTCONFIG_H_
#define NATIVEUI_GTK_UTIL_FONTCONFIG_H_

#include <fontconfig/fontconfig.h>

#include <memory>
#include <string>

#include "base/files/file_path.h"

namespace nu {

struct FcPatternDeleter {
  void operator()(FcPattern* ptr) const { FcPatternDestroy(ptr); }
};
using ScopedFcPattern = std::unique_ptr<FcPattern, FcPatternDeleter>;

// Retrieve the global font config. Must be called on the main thread.
FcConfig* GetGlobalFontConfig();

// FcPattern accessor wrappers.
std::string GetFontName(FcPattern* pattern);
std::string GetFilename(FcPattern* pattern);
int GetFontTtcIndex(FcPattern* pattern);
bool IsFontBold(FcPattern* pattern);
bool IsFontItalic(FcPattern* pattern);
bool IsFontScalable(FcPattern* pattern);
std::string GetFontFormat(FcPattern* pattern);

// Return the path of the font. Relative to the sysroot config specified in the
// font config (see: FcConfigGetSysRoot(...)).
base::FilePath GetFontPath(FcPattern* pattern);

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_FONTCONFIG_H_
