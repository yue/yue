// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

#include "base/lazy_instance.h"

namespace nu {

namespace {

Lifetime* g_lifetime = nullptr;

}  // namespace

// static
Lifetime* Lifetime::current() {
  return g_lifetime;
}

Lifetime::Lifetime() : message_loop_(base::MessageLoop::TYPE_UI),
                       weak_factory_(this) {
  DCHECK(!g_lifetime) << "Lifetime can not be created twice.";
  g_lifetime = this;
  PlatformInit();
}

Lifetime::~Lifetime() {
  g_lifetime = nullptr;
  PlatformDestroy();
}

void Lifetime::Run() {
  if (!run_loop_)
    run_loop_.reset(new base::RunLoop);
  run_loop_->Run();
  run_loop_.reset();
}

void Lifetime::Quit() {
  if (!run_loop_)
    return;
  PostTask(run_loop_->QuitClosure());
}

void Lifetime::PostTask(const base::Closure& task) {
  message_loop_.task_runner()->PostNonNestableTask(FROM_HERE, task);
}

void Lifetime::PostDelayedTask(int ms, const base::Closure& task) {
  message_loop_.task_runner()->PostNonNestableDelayedTask(
      FROM_HERE, task, base::TimeDelta::FromMilliseconds(ms));
}

}  // namespace nu
