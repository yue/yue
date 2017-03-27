// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/node_integration_win.h"

#include <windows.h>

namespace node_yue {

NodeIntegrationWin::NodeIntegrationWin() {
}

NodeIntegrationWin::~NodeIntegrationWin() {
}

void NodeIntegrationWin::PollEvents() {
  // If there are other kinds of events pending, uv_backend_timeout will
  // instruct us not to wait.
  DWORD bytes, timeout;
  ULONG_PTR key;
  OVERLAPPED* overlapped;

  timeout = uv_backend_timeout(uv_loop_);

  GetQueuedCompletionStatus(uv_loop_->iocp,
                            &bytes,
                            &key,
                            &overlapped,
                            timeout);

  // Give the event back so libuv can deal with it.
  if (overlapped != NULL)
    PostQueuedCompletionStatus(uv_loop_->iocp,
                               bytes,
                               key,
                               overlapped);
}

// static
NodeIntegration* NodeIntegration::Create() {
  return new NodeIntegrationWin();
}

}  // namespace node_yue
