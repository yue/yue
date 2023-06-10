// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/values.h"
#include "nativeui/signal.h"
#include "testing/gtest/include/gtest/gtest.h"

class SignalTest : public testing::Test {
};

TEST_F(SignalTest, MoveOnlyArguments) {
  nu::Signal<void(base::Value)> signal;
  signal.Connect([](base::Value) {});
  signal.Emit(base::Value());
}

struct Copiable {
  Copiable() {}
  Copiable(Copiable&& other) : ref(other.ref) {}
  Copiable(const Copiable& other) : ref(other.ref + 1) {}
  int ref = 1;
};

TEST_F(SignalTest, ZeroCopy) {
  nu::Signal<void(Copiable)> signal;
  signal.Connect([](Copiable arg) {
    ASSERT_EQ(arg.ref, 1);
  });
  signal.Emit(Copiable());
}
