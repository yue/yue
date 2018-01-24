// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

#include "base/logging.h"

namespace nu {

namespace {

Lifetime* g_lifetime = nullptr;

}  // namespace

// static
Lifetime* Lifetime::GetCurrent() {
  return g_lifetime;
}

Lifetime::Lifetime() : weak_factory_(this) {
  DCHECK(!g_lifetime) << "Lifetime can not be created twice.";
  g_lifetime = this;
  PlatformInit();
}

Lifetime::~Lifetime() {
  g_lifetime = nullptr;
  PlatformDestroy();
}

}  // namespace nu
