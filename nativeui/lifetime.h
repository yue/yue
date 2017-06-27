// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LIFETIME_H_
#define NATIVEUI_LIFETIME_H_

#include <memory>
#include <unordered_map>

#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "nativeui/signal.h"

#if defined(OS_MACOSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#ifdef __OBJC__
@class NUApplicationDelegate;
#else
class NUApplicationDelegate;
#endif
#endif

namespace nu {

// Manages the whole application's message loop and lifetime, should only be
// used in apps that do not have their own message loops.
class NATIVEUI_EXPORT Lifetime {
 public:
  Lifetime();
  ~Lifetime();

  static Lifetime* GetCurrent();

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

#if defined(OS_MACOSX)
  base::mac::ScopedNSAutoreleasePool autorelease_pool_;
  NUApplicationDelegate* app_delegate_;
#endif

#if defined(OS_WIN)
  static void CALLBACK OnTimer(HWND, UINT, UINT_PTR event, DWORD);

  static base::Lock lock_;
  static std::unordered_map<UINT_PTR, base::Closure> tasks_;
#endif

  base::WeakPtrFactory<Lifetime> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Lifetime);
};

}  // namespace nu

#endif  // NATIVEUI_LIFETIME_H_
