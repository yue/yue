// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_INTEGRATION_WIN_H_
#define NODE_YUE_NODE_INTEGRATION_WIN_H_

#include "node_yue/node_integration.h"

namespace node_yue {

class NodeIntegrationWin : public NodeIntegration {
 public:
  NodeIntegrationWin();
  ~NodeIntegrationWin() override;

 private:
  void PollEvents() override;
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_INTEGRATION_WIN_H_
