// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NAPI_YUE_NODE_INTEGRATION_MAC_H_
#define NAPI_YUE_NODE_INTEGRATION_MAC_H_

#include "napi_yue/node_integration.h"

namespace napi_yue {

class NodeIntegrationMac : public NodeIntegration {
 public:
  NodeIntegrationMac();
  ~NodeIntegrationMac() override;

 private:
  void PollEvents() override;
};

}  // namespace napi_yue

#endif  // NAPI_YUE_NODE_INTEGRATION_MAC_H_
