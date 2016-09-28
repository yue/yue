// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/app.h"

#include "nativeui/menu_bar.h"
#include "nativeui/state.h"

namespace nu {

// static
App* App::current() {
  return State::current()->app();
}

App::App() : message_loop_(base::MessageLoop::TYPE_UI),
             weak_factory_(this) {
  PlatformInit();
}

App::~App() {
  PlatformDestroy();
}

void App::Run() {
  run_loop_.Run();
}

void App::Quit() {
  run_loop_.Quit();
}

void App::PostTask(const base::Closure& task) {
  message_loop_.task_runner()->PostNonNestableTask(FROM_HERE, task);
}

void App::PostDelayedTask(int ms, const base::Closure& task) {
  message_loop_.task_runner()->PostNonNestableDelayedTask(
      FROM_HERE, task, base::TimeDelta::FromMilliseconds(ms));
}

void App::SetApplicationMenu(MenuBar* menu) {
  application_menu_ = menu;
  PlatformSetApplicationMenu(menu);
}

MenuBar* App::GetApplicationMenu() const {
  return application_menu_.get();
}

}  // namespace nu
