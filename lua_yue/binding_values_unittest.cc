// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <memory>
#include <string>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "lua_yue/binding_values.h"
#include "testing/gtest/include/gtest/gtest.h"

#if LUA_VERSION_NUM >= 503
# define ONE "1.0"
#else
# define ONE "1"
#endif

class YueValuesTest : public testing::Test {
 protected:
  void SetUp() override {
    lua::SetTop(state_, 0);
  }

  lua::ManagedState state_;
};

TEST_F(YueValuesTest, ValueConversions) {
  absl::optional<base::Value> in = base::JSONReader::Read(
      "{ \"a\": " ONE ",\"b\": { \"c\": [\"t\", \"e\"], \"d\": \"st\" } }");
  lua::Push(state_, *in);
  base::Value out;
  ASSERT_TRUE(lua::To(state_, 1, &out));
  std::string json;
  ASSERT_TRUE(base::JSONWriter::Write(out, &json));
  ASSERT_EQ(json, "{\"a\":" ONE ",\"b\":{\"c\":[\"t\",\"e\"],\"d\":\"st\"}}");
}
