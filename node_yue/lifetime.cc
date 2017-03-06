// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/lifetime.h"

namespace node_yue {

Lifetime::Lifetime() : lifetime_(new nu::Lifetime) {
}

Lifetime::~Lifetime() {
}

void Lifetime::Run() {
  lifetime_->Run();
}

void Lifetime::Quit() {
  lifetime_->Quit();
  lifetime_.reset();
}

}  // namespace node_yue
