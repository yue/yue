// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_BINDINGS_LINUX_H_
#define NODE_YUE_NODE_BINDINGS_LINUX_H_

#include "node_yue/node_bindings.h"

namespace node_yue {

class NodeBindingsLinux : public NodeBindings {
 public:
  NodeBindingsLinux();
  override ~NodeBindingsLinux();

 private:
  void PollEvents() override;

  // Epoll to poll for uv's backend fd.
  int epoll_;

  DISALLOW_COPY_AND_ASSIGN(NodeBindingsLinux);
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_BINDINGS_LINUX_H_
