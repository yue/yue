// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_BINDINGS_MAC_H_
#define NODE_YUE_NODE_BINDINGS_MAC_H_

#include "node_yue/node_bindings.h"

namespace node_yue {

class NodeBindingsMac : public NodeBindings {
 public:
  NodeBindingsMac();
  ~NodeBindingsMac() override;

 private:
  void PollEvents() override;

  DISALLOW_COPY_AND_ASSIGN(NodeBindingsMac);
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_BINDINGS_MAC_H_
