// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_APP_H_
#define NATIVEUI_APP_H_

#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "nativeui/nativeui_export.h"

namespace nu {

// The instance that manages message loop and App wide global APIs.
NATIVEUI_EXPORT class App {
 public:
  static App* current();

  void Run();
  void Quit();
  void PostTask(const base::Closure& task);
  void PostDelayedTask(int ms, const base::Closure& task);

  base::WeakPtr<App> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

 protected:
  App();
  virtual ~App();

 private:
  friend class State;

  base::MessageLoop message_loop_;
  base::RunLoop run_loop_;

  base::WeakPtrFactory<App> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(App);
};

}  // namespace nu

#endif  // NATIVEUI_APP_H_
