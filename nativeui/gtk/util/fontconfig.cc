// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/gtk/util/fontconfig.h"

#include "base/check_op.h"
#include "base/no_destructor.h"

namespace nu {

namespace {

// A singleton class to wrap a global font-config configuration. The
// configuration reference counter is incremented to avoid the deletion of the
// structure while being used. This class is single-threaded and should only be
// used on the UI-Thread.
class GlobalFontConfig {
 public:
  GlobalFontConfig() {
    // Without this call, the FontConfig library gets implicitly initialized
    // on the first call to FontConfig. Since it's not safe to initialize it
    // concurrently from multiple threads, we explicitly initialize it here
    // to prevent races when there are multiple renderer's querying the library:
    // http://crbug.com/404311
    // Note that future calls to FcInit() are safe no-ops per the FontConfig
    // interface.
    FcInit();

    // Increment the reference counter to avoid the config to be deleted while
    // being used (see http://crbug.com/1004254).
    fc_config_ = FcConfigGetCurrent();
    FcConfigReference(fc_config_);

    // Set rescan interval to 0 to disable re-scan. Re-scanning in the
    // background is a source of thread safety issues.
    // See in http://crbug.com/1004254.
    FcBool result = FcConfigSetRescanInterval(fc_config_, 0);
    DCHECK_EQ(result, FcTrue);
  }

  ~GlobalFontConfig() { FcConfigDestroy(fc_config_); }

  GlobalFontConfig& operator=(const GlobalFontConfig&) = delete;
  GlobalFontConfig(const GlobalFontConfig&) = delete;

  // Retrieve the native font-config FcConfig pointer.
  FcConfig* Get() const {
    DCHECK_EQ(fc_config_, FcConfigGetCurrent());
    return fc_config_;
  }

  // Retrieve the global font-config configuration.
  static GlobalFontConfig* GetInstance() {
    static base::NoDestructor<GlobalFontConfig> fontconfig;
    return fontconfig.get();
  }

 private:
  FcConfig* fc_config_ = nullptr;
};

// Extracts a string property from a font-config pattern (e.g. FcPattern).
std::string GetFontConfigPropertyAsString(FcPattern* pattern,
                                          const char* property) {
  FcChar8* text = nullptr;
  if (FcPatternGetString(pattern, property, 0, &text) != FcResultMatch ||
      text == nullptr) {
    return std::string();
  }
  return std::string(reinterpret_cast<const char*>(text));
}

// Extracts an integer property from a font-config pattern (e.g. FcPattern).
int GetFontConfigPropertyAsInt(FcPattern* pattern,
                               const char* property,
                               int default_value) {
  int value = -1;
  if (FcPatternGetInteger(pattern, property, 0, &value) != FcResultMatch)
    return default_value;
  return value;
}

// Extracts an boolean property from a font-config pattern (e.g. FcPattern).
bool GetFontConfigPropertyAsBool(FcPattern* pattern, const char* property) {
  FcBool value = FcFalse;
  if (FcPatternGetBool(pattern, property, 0, &value) != FcResultMatch)
    return false;
  return value != FcFalse;
}

}  // namespace

FcConfig* GetGlobalFontConfig() {
  return GlobalFontConfig::GetInstance()->Get();
}

std::string GetFontName(FcPattern* pattern) {
  return GetFontConfigPropertyAsString(pattern, FC_FAMILY);
}

std::string GetFilename(FcPattern* pattern) {
  return GetFontConfigPropertyAsString(pattern, FC_FILE);
}

base::FilePath GetFontPath(FcPattern* pattern) {
  std::string filename = GetFilename(pattern);

  // Obtains the system root directory in 'config' if available. All files
  // (including file properties in patterns) obtained from this 'config' are
  // relative to this system root directory.
  const char* sysroot =
      reinterpret_cast<const char*>(FcConfigGetSysRoot(nullptr));
  if (!sysroot)
    return base::FilePath(filename);

  // Paths may be specified with a heading slash (e.g.
  // /test_fonts/DejaVuSans.ttf).
  if (!filename.empty() && base::FilePath::IsSeparator(filename[0]))
    filename = filename.substr(1);

  if (filename.empty())
    return base::FilePath();

  return base::FilePath(sysroot).Append(filename);
}

int GetFontTtcIndex(FcPattern* pattern) {
  return GetFontConfigPropertyAsInt(pattern, FC_INDEX, 0);
}

bool IsFontBold(FcPattern* pattern) {
  int weight = GetFontConfigPropertyAsInt(pattern, FC_WEIGHT, FC_WEIGHT_NORMAL);
  return weight >= FC_WEIGHT_BOLD;
}

bool IsFontItalic(FcPattern* pattern) {
  int slant = GetFontConfigPropertyAsInt(pattern, FC_SLANT, FC_SLANT_ROMAN);
  return slant != FC_SLANT_ROMAN;
}

bool IsFontScalable(FcPattern* pattern) {
  return GetFontConfigPropertyAsBool(pattern, FC_SCALABLE);
}

std::string GetFontFormat(FcPattern* pattern) {
  return GetFontConfigPropertyAsString(pattern, FC_FONTFORMAT);
}

}  // namespace nu
