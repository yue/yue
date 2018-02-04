// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/debug/stack_trace.h"
#include "testing/gtest/include/gtest/gtest.h"

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  base::debug::EnableInProcessStackDumping();
  return RUN_ALL_TESTS();
}
