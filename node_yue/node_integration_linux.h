// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_INTEGRATION_LINUX_H_
#define NODE_YUE_NODE_INTEGRATION_LINUX_H_

#include "node_yue/node_integration.h"

namespace node_yue {

class NodeIntegrationLinux : public NodeIntegration {
 public:
  NodeIntegrationLinux();
  ~NodeIntegrationLinux() override;

 private:
  void PollEvents() override;

  // Epoll to poll for uv's backend fd.
  int epoll_;
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_INTEGRATION_LINUX_H_
