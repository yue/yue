// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "yue/api_message_loop.h"

#include <string>

#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"

namespace {

// Wrapping the base::MessageLoop class.
class MessageLoop : public base::RefCounted<MessageLoop> {
 public:
  static MessageLoop* Create(lua::CallContext* context,
                             const std::string& type) {
    if (type != "ui") {
      lua::PushFormatedString(context->state,
                              "MessageLoop of type '%s' is not supported",
                              type.c_str());
      context->has_error = true;
      return nullptr;
    } else if (base::MessageLoop::current()) {
      lua::Push(context->state, "MessageLoop is alraedy created");
      context->has_error = true;
      return nullptr;
    }

    return new MessageLoop(base::MessageLoop::TYPE_UI);
  }

  void Run() {
    run_loop_.Run();
  }

  void Quit() {
    run_loop_.Quit();
  }

  void PostTask(const base::Closure& task) {
    message_loop_.task_runner()->PostNonNestableTask(FROM_HERE, task);
  }

  void PostDelayedTask(int ms, const base::Closure& task) {
    message_loop_.task_runner()->PostNonNestableDelayedTask(
        FROM_HERE, task, base::TimeDelta::FromMilliseconds(ms));
  }

 private:
  friend class base::RefCounted<MessageLoop>;

  explicit MessageLoop(base::MessageLoop::Type type) : message_loop_(type) {}
  ~MessageLoop() {}

  base::MessageLoop message_loop_;
  base::RunLoop run_loop_;
};

}  // namespace

namespace lua {

template<>
struct Type<MessageLoop> {
  static constexpr const char* name = "MessageLoop";
  static void BuildMetaTable(State* state, int index) {
    RawSet(state, index,
           "new", &MessageLoop::Create,
           "run", &MessageLoop::Run,
           "quit", &MessageLoop::Quit,
           "post", &MessageLoop::PostTask,
           "postdelayed", &MessageLoop::PostDelayedTask);
  }
};

}  // namespace lua

extern "C" int luaopen_yue_message_loop(lua::State* state) {
  lua::Push(state, lua::MetaTable<MessageLoop>());
  return 1;
}
