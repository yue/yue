// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SIGNAL_H_
#define NATIVEUI_SIGNAL_H_

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

#include "base/callback.h"

namespace nu {

// A simple signal/slot implementation.
template<typename Sig> class Signal;

template<typename... Args>
class Signal<void(Args...)> {
 public:
  using Slot = base::Callback<void(Args...)>;

  int Connect(const Slot& slot) {
    slots_.push_back(std::make_tuple(++next_id_, slot));
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

  template<typename... RunArgs>
  void Emit(RunArgs&&... args) {
    // Copy the list before iterating, since it is possible that user removes
    // elements from the list when iterating.
    auto slots = slots_;
    for (auto& slot : slots)
      std::get<1>(slot).Run(std::forward(args)...);
  }

 private:
  // Use the first element of tuple as comparing key.
  static bool TupleCompare(const std::tuple<int, Slot>& element, int key) {
    return std::get<0>(element) < key;
  }

  int next_id_ = 0;
  std::vector<std::tuple<int, Slot>> slots_;
};

}  // namespace nu

#endif  // NATIVEUI_SIGNAL_H_
