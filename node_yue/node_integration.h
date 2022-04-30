// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_INTEGRATION_H_
#define NODE_YUE_NODE_INTEGRATION_H_

#include "base/memory/weak_ptr.h"
#include "uv.h"  // NOLINT
#include "v8.h"  // NOLINT

namespace nu {
class Lifetime;
}

namespace node_yue {

class NodeIntegration {
 public:
  static NodeIntegration* Create();

  virtual ~NodeIntegration();

  // Prepare for message loop integration.
  void PrepareMessageLoop();

  // Do message loop integration.
  virtual void RunMessageLoop();

 protected:
  NodeIntegration();

  // Called to poll events in new thread.
  virtual void PollEvents() = 0;

  // Run the libuv loop for once.
  void UvRunOnce();

  // Make the main thread run libuv loop.
  void WakeupMainThread();

  // Interrupt the PollEvents.
  void WakeupEmbedThread();

  // Main thread's libuv loop.
  uv_loop_t* uv_loop_;

 private:
  // Thread to poll uv events.
  static void EmbedThreadRunner(void *arg);

  // Whether the libuv loop has ended.
  bool embed_closed_;

  // Async handle used for awaking the message loop.
  uv_async_t awake_handle_;

  // Thread for polling events.
  uv_thread_t embed_thread_;

  // Semaphore to wait for main loop in the embed thread.
  uv_sem_t embed_sem_;

  base::WeakPtrFactory<NodeIntegration> weak_factory_;
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_INTEGRATION_H_
