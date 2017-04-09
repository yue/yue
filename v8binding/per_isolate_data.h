// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PER_ISOLATE_DATA_H_
#define V8BINDING_PER_ISOLATE_DATA_H_

#include <unordered_map>

#include "base/macros.h"
#include "v8.h"  // NOLINT(build/include)

namespace vb {

namespace internal {
class ObjectTracker;
}

class PerIsolateData {
 public:
  static PerIsolateData* Get(v8::Isolate* isolate);

  // Cache template objects.
  // The |name| has to be a static variable, we are using its pointer address
  // as key, instead of its string value.
  void SetFunctionTemplate(const char* name,
                           v8::Local<v8::FunctionTemplate> function_template);
  v8::Local<v8::FunctionTemplate> GetFunctionTemplate(const char* name);

  // Cache RefPtr wrappers.
  void SetObjectTracker(void* ptr, internal::ObjectTracker* wrapper);
  internal::ObjectTracker* GetObjectTracker(void* ptr);

 protected:
  explicit PerIsolateData(v8::Isolate* isolate);
  ~PerIsolateData();

 private:
  using FunctionTemplateMap =
      std::unordered_map<const char*, v8::Eternal<v8::FunctionTemplate>>;
  using ObjectTrackerMap =
      std::unordered_map<void*, internal::ObjectTracker*>;

  v8::Isolate* isolate_;
  FunctionTemplateMap function_templates_;
  ObjectTrackerMap object_trackers_;

  DISALLOW_COPY_AND_ASSIGN(PerIsolateData);
};

}  // namespace vb

#endif  // V8BINDING_PER_ISOLATE_DATA_H_
