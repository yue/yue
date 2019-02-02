// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_DRAG_DROP_CLIPBOARD_UTIL_H_
#define NATIVEUI_WIN_DRAG_DROP_CLIPBOARD_UTIL_H_

#include <windows.h>

#include <string>
#include <vector>

namespace base {
class FilePath;
}

namespace nu {

// Read filenames from HDROP.
void GetFilePathsFromHDrop(HDROP drop, std::vector<base::FilePath>* result);

// Creates a new STGMEDIUM object to hold files.
STGMEDIUM* GetStorageForFileNames(const std::vector<base::FilePath>& paths);

// Converting between MS CF_HTML format and plain text/html.
std::string HtmlToCFHtml(const std::string& html,
                         const std::string& base_url);
void CFHtmlToHtml(const std::string& cf_html,
                  std::string* html,
                  std::string* base_url);
void CFHtmlExtractMetadata(const std::string& cf_html,
                           std::string* base_url,
                           size_t* html_start,
                           size_t* fragment_start,
                           size_t* fragment_end);

}  // namespace nu

#endif  // NATIVEUI_WIN_DRAG_DROP_CLIPBOARD_UTIL_H_
