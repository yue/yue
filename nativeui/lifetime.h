// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LIFETIME_H_
#define NATIVEUI_LIFETIME_H_

#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "nativeui/signal.h"

#if defined(OS_MACOSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#ifdef __OBJC__
@class NUApplicationDelegate;
#else
class NUApplicationDelegate;
#endif
#endif

#if defined(OS_LINUX)
#include "nativeui/gtk/gtk_event_loop.h"
#endif

namespace nu {

// Manages the whole application's message loop and lifetime, should only be
// used in apps that do not have their own message loops.
class NATIVEUI_EXPORT Lifetime {
 public:
  Lifetime();
  ~Lifetime();

  static Lifetime* current();

  // Control message loop.
  void Run();
  void Quit();
  void PostTask(const base::Closure& task);
  void PostDelayedTask(int ms, const base::Closure& task);

  // Events.
  Signal<void()> on_ready;

  base::WeakPtr<Lifetime> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 private:
  void PlatformInit();
  void PlatformDestroy();

  base::MessageLoop message_loop_;
  base::RunLoop run_loop_;

#if defined(OS_MACOSX)
  base::mac::ScopedNSAutoreleasePool autorelease_pool_;
  NUApplicationDelegate* app_delegate_;
#endif

#if defined(OS_LINUX)
  GtkEventLoop gtk_event_loop_;
#endif

  base::WeakPtrFactory<Lifetime> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Lifetime);
};

}  // namespace nu

#endif  // NATIVEUI_LIFETIME_H_
