// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_ACCELERATOR_MANAGER_H_
#define NATIVEUI_ACCELERATOR_MANAGER_H_

#include <map>

#include "build/build_config.h"
#include "nativeui/nativeui_export.h"

#if defined(OS_LINUX)
typedef struct _GtkAccelGroup GtkAccelGroup;
#endif

namespace nu {

class Accelerator;
class MenuItem;

// Internal: Class reponsible for registering accelerators.
class NATIVEUI_EXPORT AcceleratorManager {
 public:
  AcceleratorManager();
  ~AcceleratorManager();

  AcceleratorManager(const AcceleratorManager&) = delete;
  AcceleratorManager& operator=(const AcceleratorManager&) = delete;

  void RegisterAccelerator(MenuItem* item, const Accelerator& accelerator);
  void RemoveAccelerator(MenuItem* item, const Accelerator& accelerator);

#if defined(OS_WIN)
  // Activate the target associated with the specified accelerator, return the
  // ID of activated item.
  int Process(const Accelerator& accelerator);
#endif

#if defined(OS_LINUX)
  GtkAccelGroup* accel_group() const { return accel_group_; }
#endif

 private:
#if defined(OS_LINUX)
  GtkAccelGroup* accel_group_;
#endif

#if defined(OS_WIN)
  using AcceleratorMap = std::map<Accelerator, int>;
  AcceleratorMap accelerators_;
#endif
};

}  // namespace nu

#endif  // NATIVEUI_ACCELERATOR_MANAGER_H_
