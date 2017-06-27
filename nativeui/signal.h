// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SIGNAL_H_
#define NATIVEUI_SIGNAL_H_

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "nativeui/nativeui_export.h"

namespace nu {

// A simple signal/slot implementation.
template<typename Sig> class SignalBase {
 public:
  using Slot = std::function<Sig>;

  int Connect(const Slot& slot) {
    slots_.push_back(std::make_pair(++next_id_, slot));
    return next_id_;
  }

  void Disconnect(int id) {
    auto iter = std::lower_bound(slots_.begin(), slots_.end(),
                                 id, TupleCompare);
    if (iter != slots_.end() && std::get<0>(*iter) == id)
      slots_.erase(iter);
  }

  void DisconnectAll() {
    slots_.clear();
  }

  bool IsEmpty() const {
    return slots_.empty();
  }

 protected:
  // Use the first element of tuple as comparing key.
  static bool TupleCompare(const std::pair<int, Slot>& element, int key) {
    return element.first < key;
  }

  int next_id_ = 0;
  std::vector<std::pair<int, Slot>> slots_;
};

template<typename Sig> class Signal;

// Signal type that does not expect return type.
template<typename... Args>
class Signal<void(Args...)> : public SignalBase<void(Args...)> {
 public:
  void Emit(Args... args) {
    // Copy the list before iterating, since it is possible that user removes
    // elements from the list when iterating.
    auto slots = this->slots_;
    for (auto& slot : slots)
      slot.second(std::forward<Args>(args)...);
  }
};

// Signal that expects boolean return value.
template<typename... Args>
class Signal<bool(Args...)> : public SignalBase<bool(Args...)> {
 public:
  bool Emit(Args... args) {
    // Copy the list before iterating, since it is possible that user removes
    // elements from the list when iterating.
    auto slots = this->slots_;
    for (auto& slot : slots) {
      if (slot.second(std::forward<Args>(args)...))
        return true;
    }
    return false;
  }
};

}  // namespace nu

#endif  // NATIVEUI_SIGNAL_H_
