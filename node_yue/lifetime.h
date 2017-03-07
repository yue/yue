// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_LIFETIME_H_
#define NODE_YUE_LIFETIME_H_

#include <memory>

#include "nativeui/nativeui.h"
#include "v8binding/v8binding.h"

namespace node_yue {

class NodeBindings;

// A wrapper of nu::Lifetime that destroys the lifetime on exit.
class Lifetime : public base::RefCounted<Lifetime> {
 public:
  Lifetime();

  void Run();
  void Quit();

 protected:
  virtual ~Lifetime();

 private:
  friend class base::RefCounted<Lifetime>;

  std::unique_ptr<nu::Lifetime> lifetime_;
  std::unique_ptr<NodeBindings> node_bindings_;
};

}  // namespace node_yue

#endif  // NODE_YUE_LIFETIME_H_
