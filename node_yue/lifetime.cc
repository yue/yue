// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/lifetime.h"

#include "node_yue/node_bindings.h"

namespace node_yue {

Lifetime::Lifetime()
    : lifetime_(new nu::Lifetime),
      node_bindings_(NodeBindings::Create()) {
  node_bindings_->PrepareMessageLoop();
}

Lifetime::~Lifetime() {
}

void Lifetime::Run() {
  node_bindings_->RunMessageLoop();
  lifetime_->Run();
}

void Lifetime::Quit() {
  lifetime_->Quit();
}

void Lifetime::PostTask(const base::Closure& task) {
  lifetime_->PostTask(task);
}

void Lifetime::PostDelayedTask(int ms, const base::Closure& task) {
  lifetime_->PostDelayedTask(ms, task);
}

}  // namespace node_yue
