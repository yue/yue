// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/node_integration.h"

#include <functional>
#include <string>
#include <vector>

#include "nativeui/message_loop.h"
#include "node.h"  // NOLINT

namespace node_yue {

NodeIntegration::NodeIntegration()
    : uv_loop_(uv_default_loop()),
      embed_closed_(false),
      weak_factory_(this) {
}

NodeIntegration::~NodeIntegration() {
  // Quit the embed thread.
  embed_closed_ = true;
  uv_sem_post(&embed_sem_);
  WakeupEmbedThread();

  // Wait for everything to be done.
  uv_thread_join(&embed_thread_);

  // Clear uv.
  uv_sem_destroy(&embed_sem_);
  uv_close(reinterpret_cast<uv_handle_t*>(&awake_handle_), nullptr);
}

void NodeIntegration::PrepareMessageLoop() {
  // Unref the handle otherwise libuv loop won't quit.
  uv_async_init(uv_loop_, &awake_handle_, nullptr);
  uv_unref(reinterpret_cast<uv_handle_t*>(&awake_handle_));

  // Start worker that will interrupt main loop when having uv events.
  uv_sem_init(&embed_sem_, 0);
  uv_thread_create(&embed_thread_, EmbedThreadRunner, this);
}

void NodeIntegration::RunMessageLoop() {
  // Run uv loop for once to give the uv__io_poll a chance to add all events.
  UvRunOnce();
}

void NodeIntegration::UvRunOnce() {
  // Deal with uv events.
  uv_run(uv_loop_, UV_RUN_NOWAIT);

  // Tell the worker thread to continue polling.
  uv_sem_post(&embed_sem_);
}

void NodeIntegration::WakeupMainThread() {
  auto self = weak_factory_.GetWeakPtr();
  nu::MessageLoop::PostTask([self] {
    if (self)
      self->UvRunOnce();
  });
}

void NodeIntegration::WakeupEmbedThread() {
  uv_async_send(&awake_handle_);
}

// static
void NodeIntegration::EmbedThreadRunner(void *arg) {
  NodeIntegration* self = static_cast<NodeIntegration*>(arg);

  while (true) {
    // Wait for the main loop to deal with events.
    uv_sem_wait(&self->embed_sem_);
    if (self->embed_closed_)
      break;

    // Wait for something to happen in uv loop.
    // Note that the PollEvents() is implemented by derived classes, so when
    // this class is being destructed the PollEvents() would not be available
    // anymore. Because of it we must make sure we only invoke PollEvents()
    // when this class is alive.
    self->PollEvents();
    if (self->embed_closed_)
      break;

    // Deal with event in main thread.
    self->WakeupMainThread();
  }
}

}  // namespace node_yue
