// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/drag_drop/clipboard_util.h"

#include <objbase.h>
#include <objidl.h>
#include <shlobj.h>
#include <shobjidl.h>

#include "base/files/file_path.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/win/scoped_hglobal.h"
#include "nativeui/gfx/canvas.h"
#include "nativeui/gfx/painter.h"
#include "nativeui/gfx/win/double_buffer.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

int ToCFType(Clipboard::Data::Type type) {
  switch (type) {
    case Clipboard::Data::Type::Text:
      return CF_UNICODETEXT;
    case Clipboard::Data::Type::HTML:
      return GetHTMLFormat();
    case Clipboard::Data::Type::Image:
      return CF_DIBV5;
    case Clipboard::Data::Type::FilePaths:
      return CF_HDROP;
    case Clipboard::Data::Type::None:
      return -1;
  }
  NOTREACHED() << "Invalid type: " << static_cast<int>(type);
  return -1;
}

UINT GetHTMLFormat() {
  static UINT html_format = ::RegisterClipboardFormat(L"HTML Format");
  return html_format;
}

bool GetFormatEtc(Clipboard::Data::Type type, FORMATETC* format) {
  int cf_type = ToCFType(type);
  if (cf_type < 0)
    return false;
  format->cfFormat = static_cast<CLIPFORMAT>(cf_type);
  format->dwAspect = DVASPECT_CONTENT;
  format->lindex = -1;
  format->tymed = TYMED_HGLOBAL;
  return true;
}

void GetFilePathsFromHDrop(HDROP drop, std::vector<base::FilePath>* result) {
  const int kMaxFilenameLen = 4096;
  const unsigned num_files = ::DragQueryFileW(drop, 0xffffffff, 0, 0);
  result->reserve(num_files);
  for (unsigned int i = 0; i < num_files; ++i) {
    wchar_t filename[kMaxFilenameLen];
    if (!::DragQueryFileW(drop, i, filename, kMaxFilenameLen))
      continue;
    result->push_back(base::FilePath(filename));
  }
}

HBITMAP GetBitmapFromImage(Image* image) {
  if (!image)
    return NULL;
  scoped_refptr<Canvas> canvas = new Canvas(image->GetSize(),
                                            image->GetScaleFactor());
  canvas->GetPainter()->DrawImage(image, RectF(image->GetSize()));
  return canvas->GetBitmap()->GetCopiedBitmap();
}

STGMEDIUM* GetStorageForFileNames(const std::vector<base::FilePath>& paths) {
  // CF_HDROP clipboard format consists of DROPFILES structure, a series of file
  // names including the terminating null character and the additional null
  // character at the tail to terminate the array.
  // For example,
  //| DROPFILES | FILENAME 1 | NULL | ... | FILENAME n | NULL | NULL |
  // For more details, please refer to
  // https://docs.microsoft.com/ko-kr/windows/desktop/shell/clipboard#cf_hdrop

  if (paths.empty())
    return nullptr;

  size_t total_bytes = sizeof(DROPFILES);
  for (const auto& filename : paths) {
    // Allocate memory of the filename's length including the null
    // character.
    total_bytes += (filename.value().length() + 1) * sizeof(wchar_t);
  }
  // |data| needs to be terminated by an additional null character.
  total_bytes += sizeof(wchar_t);

  // GHND combines GMEM_MOVEABLE and GMEM_ZEROINIT, and GMEM_ZEROINIT
  // initializes memory contents to zero.
  HANDLE hdata = ::GlobalAlloc(GHND, total_bytes);

  base::win::ScopedHGlobal<DROPFILES*> locked_mem(hdata);
  DROPFILES* drop_files = locked_mem.get();
  drop_files->pFiles = sizeof(DROPFILES);
  drop_files->fWide = TRUE;

  wchar_t* data = reinterpret_cast<wchar_t*>(
      reinterpret_cast<BYTE*>(drop_files) + sizeof(DROPFILES));

  size_t next_filename_offset = 0;
  for (const auto& filename : paths) {
    wcscpy(data + next_filename_offset, filename.value().c_str());
    // Skip the terminating null character of the filename.
    next_filename_offset += filename.value().length() + 1;
  }

  STGMEDIUM* storage = new STGMEDIUM;
  storage->tymed = TYMED_HGLOBAL;
  storage->hGlobal = hdata;
  storage->pUnkForRelease = nullptr;
  return storage;
}

// Helper method for converting from text/html to MS CF_HTML.
// Documentation for the CF_HTML format is available at
// http://msdn.microsoft.com/en-us/library/aa767917(VS.85).aspx
std::string HtmlToCFHtml(const std::string& html,
                         const std::string& base_url) {
  if (html.empty())
    return std::string();

  #define MAX_DIGITS 10
  #define MAKE_NUMBER_FORMAT_1(digits) MAKE_NUMBER_FORMAT_2(digits)
  #define MAKE_NUMBER_FORMAT_2(digits) "%0" #digits "u"
  #define NUMBER_FORMAT MAKE_NUMBER_FORMAT_1(MAX_DIGITS)

  static const char* header = "Version:0.9\r\n"
      "StartHTML:" NUMBER_FORMAT "\r\n"
      "EndHTML:" NUMBER_FORMAT "\r\n"
      "StartFragment:" NUMBER_FORMAT "\r\n"
      "EndFragment:" NUMBER_FORMAT "\r\n";
  static const char* source_url_prefix = "SourceURL:";

  static const char* start_markup =
      "<html>\r\n<body>\r\n<!--StartFragment-->";
  static const char* end_markup =
      "<!--EndFragment-->\r\n</body>\r\n</html>";

  // Calculate offsets
  size_t start_html_offset = strlen(header) - strlen(NUMBER_FORMAT) * 4 +
      MAX_DIGITS * 4;
  if (!base_url.empty()) {
    start_html_offset += strlen(source_url_prefix) +
        base_url.length() + 2;  // Add 2 for \r\n.
  }
  size_t start_fragment_offset = start_html_offset + strlen(start_markup);
  size_t end_fragment_offset = start_fragment_offset + html.length();
  size_t end_html_offset = end_fragment_offset + strlen(end_markup);

  std::string result = base::StringPrintf(header,
                                          start_html_offset,
                                          end_html_offset,
                                          start_fragment_offset,
                                          end_fragment_offset);
  if (!base_url.empty()) {
    result.append(source_url_prefix);
    result.append(base_url);
    result.append("\r\n");
  }
  result.append(start_markup);
  result.append(html);
  result.append(end_markup);

  #undef MAX_DIGITS
  #undef MAKE_NUMBER_FORMAT_1
  #undef MAKE_NUMBER_FORMAT_2
  #undef NUMBER_FORMAT

  return result;
}

// Helper method for converting from MS CF_HTML to text/html.
void CFHtmlToHtml(const std::string& cf_html,
                  std::string* html,
                  std::string* base_url) {
  size_t fragment_start = std::string::npos;
  size_t fragment_end = std::string::npos;

  CFHtmlExtractMetadata(
      cf_html, base_url, NULL, &fragment_start, &fragment_end);

  if (html &&
      fragment_start != std::string::npos &&
      fragment_end != std::string::npos) {
    *html = cf_html.substr(fragment_start, fragment_end - fragment_start);
    base::TrimWhitespaceASCII(*html, base::TRIM_ALL, html);
  }
}

void CFHtmlExtractMetadata(const std::string& cf_html,
                           std::string* base_url,
                           size_t* html_start,
                           size_t* fragment_start,
                           size_t* fragment_end) {
  // Obtain base_url if present.
  if (base_url) {
    static constexpr char kSrcUrlStr[] = "SourceURL:";
    size_t line_start = cf_html.find(kSrcUrlStr);
    if (line_start != std::string::npos) {
      size_t src_end = cf_html.find("\n", line_start);
      size_t src_start = line_start + strlen(kSrcUrlStr);
      if (src_end != std::string::npos && src_start != std::string::npos) {
        *base_url = cf_html.substr(src_start, src_end - src_start);
        base::TrimWhitespaceASCII(*base_url, base::TRIM_ALL, base_url);
      }
    }
  }

  // Find the markup between "<!--StartFragment-->" and "<!--EndFragment-->".
  // If the comments cannot be found, like copying from OpenOffice Writer,
  // we simply fall back to using StartFragment/EndFragment bytecount values
  // to determine the fragment indexes.
  std::string cf_html_lower = base::ToLowerASCII(cf_html);
  size_t markup_start = cf_html_lower.find("<html", 0);
  if (html_start) {
    *html_start = markup_start;
  }
  size_t tag_start = cf_html.find("<!--StartFragment", markup_start);
  if (tag_start == std::string::npos) {
    static constexpr char kStartFragmentStr[] = "StartFragment:";
    size_t start_fragment_start = cf_html.find(kStartFragmentStr);
    if (start_fragment_start != std::string::npos) {
      *fragment_start = static_cast<size_t>(atoi(
          cf_html.c_str() + start_fragment_start + strlen(kStartFragmentStr)));
    }

    static constexpr char kEndFragmentStr[] = "EndFragment:";
    size_t end_fragment_start = cf_html.find(kEndFragmentStr);
    if (end_fragment_start != std::string::npos) {
      *fragment_end = static_cast<size_t>(
          atoi(cf_html.c_str() + end_fragment_start + strlen(kEndFragmentStr)));
    }
  } else {
    *fragment_start = cf_html.find('>', tag_start) + 1;
    size_t tag_end = cf_html.rfind("<!--EndFragment", std::string::npos);
    *fragment_end = cf_html.rfind('<', tag_end);
  }
}

}  // namespace nu
