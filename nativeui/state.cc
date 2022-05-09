// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#include "base/lazy_instance.h"
#include "base/threading/thread_local.h"
#include "nativeui/appearance.h"
#include "nativeui/gfx/font.h"
#include "nativeui/notification_center.h"
#include "nativeui/protocol_job.h"
#include "nativeui/screen.h"
#include "third_party/yoga/Yoga.h"

#if defined(OS_WIN)
#include "base/scoped_native_library.h"
#include "base/win/scoped_com_initializer.h"
#include "nativeui/gfx/win/native_theme.h"
#include "nativeui/win/util/class_registrar.h"
#include "nativeui/win/util/gdiplus_holder.h"
#include "nativeui/win/util/scoped_ole_initializer.h"
#include "nativeui/win/util/subwin_holder.h"
#include "nativeui/win/util/timer_host.h"
#include "nativeui/win/util/tray_host.h"
#elif defined(OS_LINUX)
#include "nativeui/gfx/gtk/gtk_theme.h"
#endif

namespace nu {

namespace {

State* g_main_state = nullptr;

// A lazily created thread local storage for quick access to a thread's message
// loop, if one exists. This should be safe and free of static constructors.
base::LazyInstance<base::ThreadLocalPointer<State>>::Leaky lazy_tls_ptr =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

// static
State* State::GetCurrent() {
  return lazy_tls_ptr.Pointer()->Get();
}

// static
State* State::GetMain() {
  return g_main_state;
}

State::State() : yoga_config_(YGConfigNew()) {
  DCHECK_EQ(GetCurrent(), nullptr) << "should only have one state per thread";

  if (!g_main_state)
    g_main_state = this;
  lazy_tls_ptr.Pointer()->Set(this);
  PlatformInit();

  for (int i = 0; i < static_cast<int>(Clipboard::Type::Count); ++i)
    clipboards_[i].reset(new Clipboard(static_cast<Clipboard::Type>(i)));
}

State::~State() {
  YGConfigFree(yoga_config_);

  if (g_main_state == this)
    g_main_state = nullptr;

  DCHECK_EQ(GetCurrent(), this);
  lazy_tls_ptr.Pointer()->Set(nullptr);

  // Note that some methods may use default config which is never released.
  DCHECK_LE(YGConfigGetInstanceCount(), 1) <<
      "There are instances of YGConfig leaked on exit";
  LeakTracker<ProtocolJob>::CheckForLeaks();
}

Clipboard* State::GetClipboard(Clipboard::Type type) {
  return clipboards_[static_cast<size_t>(type)].get();
}

Screen* State::GetScreen() {
  if (!screen_)
    screen_.reset(new Screen);
  return screen_.get();
}

Appearance* State::GetAppearance() {
  if (!appearance_)
    appearance_.reset(new Appearance);
  return appearance_.get();
}

NotificationCenter* State::GetNotificationCenter() {
  if (!notification_center_)
    notification_center_.reset(new NotificationCenter);
  return notification_center_.get();
}

}  // namespace nu
