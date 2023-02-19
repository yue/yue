// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NAPI_YUE_NODE_INTEGRATION_LINUX_H_
#define NAPI_YUE_NODE_INTEGRATION_LINUX_H_

#include "napi_yue/node_integration.h"

namespace napi_yue {

class NodeIntegrationLinux : public NodeIntegration {
 public:
  NodeIntegrationLinux();
  ~NodeIntegrationLinux() override;

 private:
  void PollEvents() override;

  // Epoll to poll for uv's backend fd.
  int epoll_;
};

}  // namespace napi_yue

#endif  // NAPI_YUE_NODE_INTEGRATION_LINUX_H_
