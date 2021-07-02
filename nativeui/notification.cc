// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/notification.h"

#include <utility>

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

}  // namespace nu
