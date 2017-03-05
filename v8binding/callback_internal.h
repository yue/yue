// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_CALLBACK_INTERNAL_H_
#define V8BINDING_CALLBACK_INTERNAL_H_

#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "v8binding/arguments.h"
#include "v8binding/template_util.h"

namespace vb {

enum CreateFunctionTemplateFlags {
  HolderIsFirstArgument = 1 << 0,
};

namespace internal {

template<typename T>
struct CallbackParamTraits {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T&> {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T*> {
  typedef T* LocalType;
};

// CallbackHolder and CallbackHolderBase are used to pass a base::Callback from
// CreateFunctionTemplate through v8 (via v8::FunctionTemplate) to
// DispatchToCallback, where it is invoked.

// This simple base class is used so that we can share a single object template
// among every CallbackHolder instance.
class CallbackHolderBase {
 public:
  v8::Local<v8::External> GetHandle(v8::Isolate* isolate);

 protected:
  explicit CallbackHolderBase(v8::Isolate* isolate);
  virtual ~CallbackHolderBase();

 private:
  static void FirstWeakCallback(
      const v8::WeakCallbackInfo<CallbackHolderBase>& data);
  static void SecondWeakCallback(
      const v8::WeakCallbackInfo<CallbackHolderBase>& data);

  v8::Global<v8::External> v8_ref_;

  DISALLOW_COPY_AND_ASSIGN(CallbackHolderBase);
};

template<typename Sig>
class CallbackHolder : public CallbackHolderBase {
 public:
  CallbackHolder(v8::Isolate* isolate,
                 const base::Callback<Sig>& callback,
                 int flags)
      : CallbackHolderBase(isolate), callback(callback), flags(flags) {}

  base::Callback<Sig> callback;
  int flags;

 private:
  virtual ~CallbackHolder() {}

  DISALLOW_COPY_AND_ASSIGN(CallbackHolder);
};

template<typename T>
bool GetNextArgument(Arguments* args, int create_flags, bool is_first,
                     T* result) {
  if (is_first && (create_flags & HolderIsFirstArgument) != 0) {
    return args->GetHolder(result);
  } else {
    return args->GetNext(result);
  }
}

// For advanced use cases, we allow callers to request the unparsed Arguments
// object and poke around in it directly.
inline bool GetNextArgument(Arguments* args, int create_flags, bool is_first,
                            Arguments* result) {
  *result = *args;
  return true;
}
inline bool GetNextArgument(Arguments* args, int create_flags, bool is_first,
                            Arguments** result) {
  *result = args;
  return true;
}

// It's common for clients to just need the isolate, so we make that easy.
inline bool GetNextArgument(Arguments* args, int create_flags,
                            bool is_first, v8::Isolate** result) {
  *result = args->isolate();
  return true;
}

// Class template for extracting and storing single argument for callback
// at position |index|.
template<size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType value;
  bool ok;

  ArgumentHolder(Arguments* args, int create_flags)
      : ok(GetNextArgument(args, create_flags, index == 0, &value)) {
    if (!ok) {
      // Ideally we would include the expected c++ type in the error
      // message which we can access via typeid(ArgType).name()
      // however we compile with no-rtti, which disables typeid.
      args->ThrowError();
    }
  }
};

// Class template for converting arguments from JavaScript to C++ and running
// the callback with them.
template<typename IndicesType, typename... ArgTypes>
class Invoker {};

template<size_t... indices, typename... ArgTypes>
class Invoker<IndicesHolder<indices...>, ArgTypes...>
    : public ArgumentHolder<indices, ArgTypes>... {
 public:
  // Invoker<> inherits from ArgumentHolder<> for each argument.
  // C++ has always been strict about the class initialization order,
  // so it is guaranteed ArgumentHolders will be initialized (and thus, will
  // extract arguments from Arguments) in the right order.
  Invoker(Arguments* args, int create_flags)
      : ArgumentHolder<indices, ArgTypes>(args, create_flags)..., args_(args) {
    // GCC thinks that create_flags is going unused, even though the
    // expansion above clearly makes use of it. Per jyasskin@, casting
    // to void is the commonly accepted way to convince the compiler
    // that you're actually using a parameter/varible.
    (void)create_flags;
  }

  bool IsOK() {
    return And(ArgumentHolder<indices, ArgTypes>::ok...);
  }

  template<typename ReturnType>
  void DispatchToCallback(base::Callback<ReturnType(ArgTypes...)> callback) {
    v8::MicrotasksScope script_scope(
        args_->isolate(), v8::MicrotasksScope::kRunMicrotasks);
    args_->Return(callback.Run(ArgumentHolder<indices, ArgTypes>::value...));
  }

  // In C++, you can declare the function foo(void), but you can't pass a void
  // expression to foo. As a result, we must specialize the case of Callbacks
  // that have the void return type.
  void DispatchToCallback(base::Callback<void(ArgTypes...)> callback) {
    v8::MicrotasksScope script_scope(
        args_->isolate(), v8::MicrotasksScope::kRunMicrotasks);
    callback.Run(ArgumentHolder<indices, ArgTypes>::value...);
  }

 private:
  static bool And() { return true; }
  template<typename... T>
  static bool And(bool arg1, T... args) {
    return arg1 && And(args...);
  }

  Arguments* args_;
};

// DispatchToCallback converts all the JavaScript arguments to C++ types and
// invokes the base::Callback.
template <typename Sig>
struct Dispatcher {};

template <typename ReturnType, typename... ArgTypes>
struct Dispatcher<ReturnType(ArgTypes...)> {
  static void DispatchToCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    Arguments args(info);
    v8::Local<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<ReturnType(ArgTypes...)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
    Invoker<Indices, ArgTypes...> invoker(&args, holder->flags);
    if (invoker.IsOK())
      invoker.DispatchToCallback(holder->callback);
  }
};

}  // namespace internal

}  // namespace vb

#endif  // V8BINDING_CALLBACK_INTERNAL_H_
