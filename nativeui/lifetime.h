// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LIFETIME_H_
#define NATIVEUI_LIFETIME_H_

#include <vector>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "nativeui/signal.h"

#if defined(OS_MAC)
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

#if defined(OS_MAC)
  enum class Reply {
    Success = 0,
    Cancel  = 1,
    Failure = 2,
  };
#endif

  // Events.
#if defined(OS_MAC)
  Signal<void()> on_ready;
  Signal<void()> on_activate;
#endif

  // Delegate methods.
#if defined(OS_MAC)
  std::function<Reply(std::vector<base::FilePath>)> open_files;
#endif

  base::WeakPtr<Lifetime> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 private:
  void PlatformInit();
  void PlatformDestroy();

#if defined(OS_MAC)
  base::mac::ScopedNSAutoreleasePool autorelease_pool_;
  NUApplicationDelegate* app_delegate_;
#endif

  base::WeakPtrFactory<Lifetime> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_LIFETIME_H_
