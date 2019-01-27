// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/clipboard.h"

namespace nu {

Clipboard::Clipboard(Type type)
    : type_(type), clipboard_(PlatformCreate(type)), weak_factory_(this) {}

Clipboard::~Clipboard() {
  PlatformDestroy();
}

}  // namespace nu
