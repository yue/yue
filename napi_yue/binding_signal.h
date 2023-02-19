// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NAPI_YUE_BINDING_SIGNAL_H_
#define NAPI_YUE_BINDING_SIGNAL_H_

#include <utility>

#include "nativeui/nativeui.h"
#include "third_party/kizunapi/kizunapi.h"

namespace ki {

template<typename Sig>
struct Type<nu::SignalBase<Sig>> {
  static constexpr const char* name = "Signal";
  static void Define(napi_env env,
                     napi_value constructor,
                     napi_value prototype) {
    Set(env, prototype,
        "connect",
        WrapMethod(&nu::SignalBase<Sig>::Connect, [](Arguments args,
                                                     napi_value ret) {
          Map slots;
          napi_value signal_name;
          if (!GetInfo(args, &slots, &signal_name))
            return;
          // signal.responder.slots[signal.name][id] = slot
          slots.GetOrCreateMap(signal_name).Set(ret, args[0]);
        }),
        "disconnect",
        WrapMethod(&nu::SignalBase<Sig>::Disconnect, [](Arguments args) {
          Map slots;
          napi_value signal_name;
          if (!GetInfo(args, &slots, &signal_name))
            return;
          // delete signal.responder.slots[signal.name][id]
          slots.GetOrCreateMap(signal_name).Delete(args[0]);
        }),
        "disconnectAll",
        WrapMethod(&nu::SignalBase<Sig>::DisconnectAll, [](Arguments args) {
          Map slots;
          napi_value signal_name;
          if (!GetInfo(args, &slots, &signal_name))
            return;
          // delete signal.responder.slots[signal.name]
          slots.Delete(signal_name);
        }),
        "isEmpty", &nu::SignalBase<Sig>::IsEmpty);
  }
  static bool GetInfo(const ki::Arguments& args,
                      Map* slots,
                      napi_value* signal_name) {
    AttachedTable table(args);
    AttachedTable responder;
    if (table.Get("responder", &responder) &&
        table.Get("name", signal_name)) {
      *slots = responder.GetOrCreateMap("slots");
      return true;
    }
    assert(false);
    napi_throw_error(args.Env(), nullptr, "Can not find responder info");
    return false;
  }
};

template<typename Sig>
struct TypeBridge<nu::SignalBase<Sig>> {
  static nu::SignalBase<Sig>* Wrap(nu::SignalBase<Sig>* ptr) {
    return ptr;
  }
  static void Finalize(nu::SignalBase<Sig>* data) {
    // Memory is managed on C++ side.
  }
};

}  // namespace ki

namespace napi_yue {

// Helper to get types from pointer to signal member.
template<typename T>
struct SignalTraits {
  using Type = typename ki::internal::ExtractMemberPointer<T>::MemberType::Base;
  using ClassType = typename ki::internal::ExtractMemberPointer<T>::ClassType;
};

template<typename T>
inline napi_value SignalGetter(typename SignalTraits<T>::ClassType* p,
                               ki::Arguments args,
                               const char* name,
                               T signal_ptr) {
  // The |signal_ptr| is a pointer to member data, uses pointer to the signal
  // itself for conversion.
  napi_value signal = ki::ToNode(
      args.Env(),
      static_cast<typename SignalTraits<T>::Type*>(&(p->*signal_ptr)));
  // signal.responder = this
  ki::Map table = ki::AttachedTable(args.Env(), signal);
  table.Set("name", name);
  table.Set("responder", args.This());
  return signal;
}

inline void SignalSetter(ki::Arguments args, const char* name) {
  // signal = this[name]
  napi_value signal;
  if (!ki::Get(args.Env(), args.This(), name, &signal)) {
    assert(false);
    return;
  }
  // signal.connect(args[0])
  ki::CallMethod(args.Env(), signal, "connect", args[0]);
}

// A helper to setup signals on a class.
// The |name| must be a string literal as we are stroing it as const char*, and
// it seems overkill to employ std::string here.
template<typename T, size_t n>
inline ki::Property Signal(const char (&name)[n], T signal_ptr) {
  using ClassType = typename SignalTraits<T>::ClassType;
  std::function<napi_value(ClassType*, ki::Arguments)> getter =
      std::bind(SignalGetter<T>,
                std::placeholders::_1, std::placeholders::_2, name, signal_ptr);
  std::function<void(ki::Arguments)> setter =
      std::bind(SignalSetter, std::placeholders::_1, name);
  return ki::Property(name,
                      // Signals are usualy used and discarded.
                      ki::Property::CacheMode::NoCache,
                      ki::Getter(getter, ki::HolderIsFirstArgument),
                      ki::Setter(setter, ki::FunctionArgumentIsWeakRef));
}

// A helper to setup delegates on a class.
template<typename T>
inline ki::Property Delegate(const char* name, T delegate_ptr) {
  return ki::Property(name,
                      // Note that the cache here is required as we are relying
                      // on it to keep ref of the callback.
                      ki::Property::CacheMode::GetterAndSetter,
                      ki::Getter(delegate_ptr),
                      ki::Setter(delegate_ptr, ki::FunctionArgumentIsWeakRef));
}

}  // namespace napi_yue

#endif  // NAPI_YUE_BINDING_SIGNAL_H_
