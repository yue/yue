// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "lua_yue/binding_signal.h"

#include "lua_yue/builtin_loader.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class YueSignalTest : public testing::Test {
 protected:
  void SetUp() override {
    luaL_openlibs(state_);
    yue::InsertBuiltinModuleLoader(state_);
    luaL_dostring(state_, "win = require('yue.gui').Window.create{}");
  }

  lua::ManagedState state_;
};

void OnClose(bool* ptr) {
  *ptr = true;
}

TEST_F(YueSignalTest, Connect) {
  bool closed = false;
  std::function<void()> on_close = [&closed] { closed = true; };
  lua::Push(state_, on_close);
  lua_setglobal(state_, "callback");
  ASSERT_FALSE(luaL_dostring(state_,
      "win.onclose:connect(callback)\n"
      "win:close()"));
  EXPECT_TRUE(closed);
}

TEST_F(YueSignalTest, ConnectCheckParam) {
  ASSERT_TRUE(luaL_dostring(state_, "win.onclose:connect(123)\n"));
  std::string error;
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "error converting arg at index 2 from number to function");
}

TEST_F(YueSignalTest, Disconnect) {
  bool closed = false;
  std::function<void()> on_close = [&closed] { closed = true; };
  lua::Push(state_, on_close);
  lua_setglobal(state_, "callback");
  ASSERT_FALSE(luaL_dostring(state_,
      "local id = win.onclose:connect(callback)\n"
      "win.onclose:disconnect(id)\n"
      "win:close()"));
  EXPECT_FALSE(closed);
}

TEST_F(YueSignalTest, DisconnectAll) {
  bool closed = false;
  std::function<void()> on_close = [&closed] { closed = true; };
  lua::Push(state_, on_close);
  lua_setglobal(state_, "callback");
  ASSERT_FALSE(luaL_dostring(state_,
      "local id = win.onclose:connect(callback)\n"
      "win.onclose:disconnectall()\n"
      "win:close()"));
  EXPECT_FALSE(closed);
}

TEST_F(YueSignalTest, Cached) {
  ASSERT_FALSE(luaL_dostring(state_, "return win.onclose"));
  ASSERT_FALSE(luaL_dostring(state_, "return win.onclose"));
  EXPECT_TRUE(lua::Compare(state_, 1, 2, lua::CompareOp::EQ));
}

TEST_F(YueSignalTest, OwnerGarbageCollected) {
  ASSERT_FALSE(luaL_dostring(state_,
      "signal = win.onclose\n"
      "win = nil"));
  lua::CollectGarbage(state_);
  std::string error;
  ASSERT_TRUE(luaL_dostring(state_, "signal:disconnect(1)\n"));
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "Owner of signal is gone");
  ASSERT_TRUE(luaL_dostring(state_, "signal:connect(function() end)\n"));
  ASSERT_TRUE(lua::Pop(state_, &error));
  EXPECT_EQ(error, "Owner of signal is gone");
}

TEST_F(YueSignalTest, EventAssignment) {
  bool closed = false;
  std::function<void()> on_close = [&closed] { closed = true; };
  lua::Push(state_, on_close);
  lua_setglobal(state_, "callback");
  ASSERT_FALSE(luaL_dostring(state_,
      "win.onclose = callback\n"
      "win:close()"));
  EXPECT_TRUE(closed);
}

TEST_F(YueSignalTest, NilAssignment) {
  bool closed = false;
  std::function<void()> on_close = [&closed] { closed = true; };
  lua::Push(state_, on_close);
  lua_setglobal(state_, "callback");
  ASSERT_FALSE(luaL_dostring(state_,
      "win.shouldclose = callback\n"
      "win.shouldclose = nil\n"
      "win:close()"));
  EXPECT_FALSE(closed);
}

TEST_F(YueSignalTest, DelegateAssignment) {
  bool closed = false;
  std::function<void()> on_close = [&closed] { closed = true; };
  lua::Push(state_, on_close);
  lua_setglobal(state_, "callback");
  ASSERT_FALSE(luaL_dostring(state_,
      "win.onclose = callback\n"
      "win.shouldclose = function() return false end\n"
      "win:close()"));
  EXPECT_FALSE(closed);
  ASSERT_FALSE(luaL_dostring(state_,
      "win.shouldclose = nil\n"
      "win:close()"));
  EXPECT_TRUE(closed);
}

TEST_F(YueSignalTest, SignalReference) {
  ASSERT_FALSE(luaL_dostring(state_,
      // Weak table.
      "t = {}\n"
      "setmetatable(t, { __mode = 'v' })\n"
      // Put function to weak table and signal.
      "c = function() end\n"
      "t.c = c\n"
      "win.onclose = c\n"
      // It should live through grabage collection.
      "c = nil\n"
      "collectgarbage()\n"
      "assert(t.c)\n"));
  ASSERT_FALSE(luaL_dostring(state_,
      // It should disappear after removing ref.
      "win.onclose:disconnectall()\n"
      "collectgarbage()\n"
      "assert(t.c == nil)\n"));
}

TEST_F(YueSignalTest, SignalCyclicReference) {
  ASSERT_FALSE(luaL_dostring(state_,
      // Weak table.
      "t = {}\n"
      "setmetatable(t, { __mode = 'v' })\n"
      // Wrapper object includes win.
      "w = {}\n"
      "w.window = w\n"
      "w.draw = function() w.window.getbounds() end\n"
      "win.onclose:connect(function() w.draw() end)\n"
      // Put object to weak table.
      "t.w = w\n"
      // Wrapper object and win can be garbage collected.
      "w = nil\n"
      "win = nil\n"
      "collectgarbage()\n"
      "assert(t.w == nil)\n"));
}

struct Copiable {
  explicit Copiable(int ref = 1) : ref(ref) {}
  Copiable(const Copiable& other) : ref(other.ref + 1) {}
  Copiable(Copiable&& other) : ref(other.ref) {
    other.ref = 0;
  }
  Copiable& operator=(Copiable&& other) {
    ref = other.ref;
    other.ref = 0;
    return *this;
  }
  int ref;
};

class TestClass : public base::RefCounted<TestClass> {
 public:
  TestClass() {}

  void Emit() {
    signal.Emit(Copiable());
  }

  nu::Signal<void(Copiable)> signal;

 protected:
  friend class base::RefCounted<TestClass>;

  virtual ~TestClass() {}
};

namespace lua {

template<>
struct Type<Copiable> {
  static constexpr const char* name = "Arg";
  static inline void Push(State* state, Copiable arg) {
    lua_pushinteger(state, arg.ref);
  }
  static inline bool To(State* state, int index, Copiable* out) {
    int success = 0;
    int ret = lua_tointegerx(state, index, &success);
    if (success)
      *out = Copiable(ret);
    return success != 0;
  }
};

template<>
struct Type<TestClass> {
  static constexpr const char* name = "TestClass";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &CreateOnHeap<TestClass>,
           "emit", &TestClass::Emit);
    RawSetProperty(state, index, "signal", &TestClass::signal);
  }
};

}  // namespace lua

TEST_F(YueSignalTest, FunctionCallZeroCopyArg) {
  scoped_refptr<TestClass> instance = new TestClass;
  lua::Push(state_, instance.get());  // 1

  bool called = false;
  std::function<void(Copiable)> handler = [&called](Copiable arg) {
    ASSERT_EQ(arg.ref, 1);
    called = true;
  };
  lua::Push(state_, handler);  // 2

  ASSERT_TRUE(lua::PSet(state_, 1,
                        "signal", lua::ValueOnStack(state_, 2)));  // 3

  ASSERT_TRUE(lua::PGet(state_, 1, "emit"));
  ASSERT_TRUE(lua::PCall(state_, nullptr, lua::ValueOnStack(state_, 1)));
  ASSERT_TRUE(called);
}
