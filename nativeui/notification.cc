// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/notification.h"

#include <utility>

#include "base/logging.h"
#include "nativeui/gfx/image.h"

namespace nu {

Notification::Notification() {
  PlatformInit();
}

Notification::~Notification() {
  PlatformDestroy();
}

void Notification::SetImage(scoped_refptr<Image> image) {
  PlatformSetImage(image.get());
  image_ = std::move(image);
}

#if defined(OS_LINUX) || defined(OS_WIN)
bool Notification::WriteImageToTempDir(Image* image, base::FilePath* out) {
  if (!image_dir_.CreateUniqueTempDir()) {
    LOG(ERROR) << "Failed for create temporary dir";
    return false;
  }
  base::FilePath path =
      image_dir_.GetPath().Append(FILE_PATH_LITERAL("image.png"));
  if (!image->WriteToFile("png", path)) {
    LOG(ERROR) << "Failed to write image to disk";
    return false;
  }
  *out = std::move(path);
  return true;
}

void Notification::PlatformSetImage(Image* image) {
  base::FilePath path;
  if (!WriteImageToTempDir(image, &path))
    return;
  SetImagePath(path);
}
#endif

}  // namespace nu
