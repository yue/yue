// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/node_integration_linux.h"

#include <sys/epoll.h>

namespace node_yue {

NodeIntegrationLinux::NodeIntegrationLinux() : epoll_(epoll_create(1)) {
  int backend_fd = uv_backend_fd(uv_loop_);
  struct epoll_event ev = { 0 };
  ev.events = EPOLLIN;
  ev.data.fd = backend_fd;
  epoll_ctl(epoll_, EPOLL_CTL_ADD, backend_fd, &ev);
}

NodeIntegrationLinux::~NodeIntegrationLinux() {
}

void NodeIntegrationLinux::PollEvents() {
  int timeout = uv_backend_timeout(uv_loop_);

  // Wait for new libuv events.
  int r;
  do {
    struct epoll_event ev;
    r = epoll_wait(epoll_, &ev, 1, timeout);
  } while (r == -1 && errno == EINTR);
}

// static
NodeIntegration* NodeIntegration::Create() {
  return new NodeIntegrationLinux();
}

}  // namespace node_yue
