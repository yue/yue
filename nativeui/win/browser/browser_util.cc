// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/browser/browser_util.h"

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/win/registry.h"

namespace nu {

namespace {

#define IE_FEATURE_CONTROL \
    L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\"

const DWORD IE_VERSION = 11000;

inline void WriteKey(const wchar_t* exe, const wchar_t* key, DWORD value) {
  base::win::RegKey(HKEY_CURRENT_USER, key, KEY_ALL_ACCESS)
      .WriteValue(exe, IE_VERSION);
}

}  // namespace

void FixIECompatibleMode() {
  base::FilePath exe_path;
  if (!PathService::Get(base::FILE_EXE, &exe_path))
    return;
  base::string16 exe_name = exe_path.BaseName().value();

  // Windows Internet Explorer 8 and later. The FEATURE_BROWSER_EMULATION
  // feature defines the default emulation mode for Internet Explorer and
  // supports the following values.
  // Webpages containing standards-based !DOCTYPE directives are displayed
  // in IE10 Standards mode.
  WriteKey(exe_name.c_str(),
           IE_FEATURE_CONTROL L"FEATURE_BROWSER_EMULATION", IE_VERSION);

  // Internet Explorer 8 or later. The FEATURE_AJAX_CONNECTIONEVENTS feature
  // enables events that occur when the value of the online property of the
  // navigator object changes, such as when the user chooses to work offline.
  // For more information, see the ononline and onoffline events.
  WriteKey(exe_name.c_str(),
           IE_FEATURE_CONTROL L"FEATURE_AJAX_CONNECTIONEVENTS", 1);

  // Internet Explorer 9. The FEATURE_GPU_RENDERING feature enables Internet
  // Explorer to use a graphics processing unit (GPU) to render content. This
  // dramatically improves performance for webpages that are rich in graphics.
  WriteKey(exe_name.c_str(),
           IE_FEATURE_CONTROL L"FEATURE_GPU_RENDERING", 1);

  // Internet Explorer 8 and later. When enabled, the
  // FEATURE_DISABLE_NAVIGATION_SOUNDS feature disables the sounds played when
  // you open a link in a webpage.
  WriteKey(exe_name.c_str(),
           IE_FEATURE_CONTROL L"FEATURE_DISABLE_NAVIGATION_SOUNDS", 1);

  // When enabled, the FEATURE_STATUS_BAR_THROTTLING feature limits the
  // frequency of status bar updates to one update every 200 milliseconds.
  WriteKey(exe_name.c_str(),
           IE_FEATURE_CONTROL L"FEATURE_STATUS_BAR_THROTTLING", 1);
}

}  // namespace nu
