// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_LOCKER_H_
#define V8BINDING_LOCKER_H_

#include <memory>

namespace v8 {
class Isolate;
class Locker;
}

namespace vb {

// Only lock when lockers are used in current thread.
class Locker {
 public:
  explicit Locker(v8::Isolate* isolate);
  ~Locker();

 private:
  void* operator new(size_t size);
  void operator delete(void*, size_t);

  std::unique_ptr<v8::Locker> locker_;
};

}  // namespace vb

#endif  // V8BINDING_LOCKER_H_
