// Copyright 2018 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/protocol_file_job.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

namespace nu {

namespace {

struct MimeInfo {
  const char* const mime_type;

  // Comma-separated list of possible extensions for the type. The first
  // extension is considered preferred.
  const char* const extensions;
};

// Order of entries in the following mapping lists matters only when the same
// extension is shared between multiple MIME types.

static const MimeInfo kPrimaryMappings[] = {
    // Must precede audio/webm .
    {"video/webm", "webm"},

    {"application/javascript", "js"},
    {"application/wasm", "wasm"},
    {"application/x-chrome-extension", "crx"},
    {"application/xhtml+xml", "xhtml,xht,xhtm"},
    {"audio/flac", "flac"},
    {"audio/mp3", "mp3"},
    {"audio/ogg", "ogg,oga,opus"},
    {"audio/wav", "wav"},
    {"audio/webm", "webm"},
    {"audio/x-m4a", "m4a"},
    {"image/gif", "gif"},
    {"image/jpeg", "jpeg,jpg"},
    {"image/png", "png"},
    {"image/webp", "webp"},
    {"multipart/related", "mht,mhtml"},
    {"text/css", "css"},
    {"text/html", "html,htm,shtml,shtm"},
    {"text/xml", "xml"},
    {"video/mp4", "mp4,m4v"},
    {"video/ogg", "ogv,ogm"},
};

static const MimeInfo kSecondaryMappings[] = {
    // Must precede image/vnd.microsoft.icon .
    {"image/x-icon", "ico"},

    {"application/epub+zip", "epub"},
    {"application/font-woff", "woff"},
    {"application/gzip", "gz,tgz"},
    {"application/json", "json"},  // Per http://www.ietf.org/rfc/rfc4627.txt.
    {"application/octet-stream", "bin,exe,com"},
    {"application/pdf", "pdf"},
    {"application/pkcs7-mime", "p7m,p7c,p7z"},
    {"application/pkcs7-signature", "p7s"},
    {"application/postscript", "ps,eps,ai"},
    {"application/rdf+xml", "rdf"},
    {"application/rss+xml", "rss"},
    {"application/vnd.android.package-archive", "apk"},
    {"application/vnd.mozilla.xul+xml", "xul"},
    {"application/x-gzip", "gz,tgz"},
    {"application/x-mpegurl", "m3u8"},
    {"application/x-shockwave-flash", "swf,swl"},
    {"application/x-tar", "tar"},
    {"application/zip", "zip"},
    {"audio/mpeg", "mp3"},
    {"image/bmp", "bmp"},
    {"image/jpeg", "jfif,pjpeg,pjp"},
    {"image/svg+xml", "svg,svgz"},
    {"image/tiff", "tiff,tif"},
    {"image/vnd.microsoft.icon", "ico"},
    {"image/x-png", "png"},
    {"image/x-xbitmap", "xbm"},
    {"message/rfc822", "eml"},
    {"text/calendar", "ics"},
    {"text/html", "ehtml"},
    {"text/plain", "txt,text"},
    {"text/x-sh", "sh"},
    {"text/xml", "xsl,xbl,xslt"},
    {"video/mpeg", "mpeg,mpg"},
};

// Finds mime type of |ext| from |mappings|.
template <size_t num_mappings>
const char* FindMimeType(const MimeInfo (&mappings)[num_mappings],
                         const std::string& ext) {
  for (const auto& mapping : mappings) {
    const char* extensions = mapping.extensions;
    for (;;) {
      size_t end_pos = strcspn(extensions, ",");
      // The length check is required to prevent the StringPiece below from
      // including uninitialized memory if ext is longer than extensions.
      if (end_pos == ext.size() &&
          base::EqualsCaseInsensitiveASCII(
              base::StringPiece(extensions, ext.size()), ext)) {
        return mapping.mime_type;
      }
      extensions += end_pos;
      if (!*extensions)
        break;
      extensions += 1;  // skip over comma
    }
  }
  return nullptr;
}

bool GetMimeTypeFromExtension(const base::FilePath::StringType& ext,
                              std::string* result) {
  // Avoids crash when unable to handle a long file path. See crbug.com/48733.
  const unsigned kMaxFilePathSize = 65536;
  if (ext.length() > kMaxFilePathSize)
    return false;

  // Reject a string which contains null character.
  base::FilePath::StringType::size_type nul_pos =
      ext.find(FILE_PATH_LITERAL('\0'));
  if (nul_pos != base::FilePath::StringType::npos)
    return false;

  // We implement the same algorithm as Mozilla for mapping a file extension to
  // a mime type.  That is, we first check a hard-coded list (that cannot be
  // overridden), and then if not found there, we defer to the system registry.
  // Finally, we scan a secondary hard-coded list to catch types that we can
  // deduce but that we also want to allow the OS to override.

  base::FilePath path_ext(ext);
  const std::string ext_narrow_str = path_ext.AsUTF8Unsafe();
  const char* mime_type = FindMimeType(kPrimaryMappings, ext_narrow_str);
  if (mime_type) {
    *result = mime_type;
    return true;
  }

  mime_type = FindMimeType(kSecondaryMappings, ext_narrow_str);
  if (mime_type) {
    *result = mime_type;
    return true;
  }

  return false;
}

}  // namespace

ProtocolFileJob::ProtocolFileJob(const base::FilePath& path)
    : path_(path),
      file_(path, base::File::FLAG_OPEN | base::File::FLAG_READ),
      content_length_(file_.IsValid() ? file_.GetLength() : 0) {
}

ProtocolFileJob::~ProtocolFileJob() {
}

bool ProtocolFileJob::Start() {
  if (!file_.IsValid())
    return false;
  notify_content_length(content_length_);
  return true;
}

void ProtocolFileJob::Kill() {
  file_.Close();
}

bool ProtocolFileJob::GetMimeType(std::string* mime_type) {
  base::FilePath::StringType ext = path_.Extension();
  if (ext.empty())
    return false;
  return GetMimeTypeFromExtension(ext.substr(1), mime_type);
}

size_t ProtocolFileJob::Read(void* buf, size_t buf_size) {
  if (content_length_ == 0)
    return 0;
  if (content_length_ < static_cast<int64_t>(buf_size))
    buf_size = content_length_;
  int nread = file_.ReadAtCurrentPos(static_cast<char*>(buf),
                                     static_cast<int>(buf_size));
  if (nread > 0) {
    content_length_ -= nread;
    return nread;
  } else {
    content_length_ = 0;
    return 0;
  }
}

}  // namespace nu
