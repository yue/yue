// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_BINDINGS_WIN_H_
#define NODE_YUE_NODE_BINDINGS_WIN_H_

#include "node_yue/node_bindings.h"

namespace node_yue {

class NodeBindingsWin : public NodeBindings {
 public:
  NodeBindingsWin();
  virtual ~NodeBindingsWin();

 private:
  void PollEvents() override;

  DISALLOW_COPY_AND_ASSIGN(NodeBindingsWin);
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_BINDINGS_WIN_H_
