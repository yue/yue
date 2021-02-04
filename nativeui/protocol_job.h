// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PROTOCOL_JOB_H_
#define NATIVEUI_PROTOCOL_JOB_H_

#include <functional>
#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/util/leak_tracker.h"

namespace nu {

// A simple class used by Browser to serve custom protocol requests.
class NATIVEUI_EXPORT ProtocolJob : public base::RefCounted<ProtocolJob> {
 public:
  // Subclasses should implement this.
  virtual bool Start();
  virtual void Kill();
  virtual bool GetMimeType(std::string* mime_type) = 0;
  virtual size_t Read(void* buf, size_t buf_size) = 0;

  // Internal: Used by Browser implementations to plug adapters.
  void Plug(std::function<void(int)> start);

 protected:
  friend class base::RefCounted<ProtocolJob>;

  ProtocolJob();
  virtual ~ProtocolJob();

  // Used by subclasses to notify the browser.
  std::function<void(int)> notify_content_length;

  LeakTracker<ProtocolJob> leak_tracker_;
};

// Use string as response.
class NATIVEUI_EXPORT ProtocolStringJob : public ProtocolJob {
 public:
  ProtocolStringJob(const std::string& mime_type, const std::string& content);

  bool Start() override;
  bool GetMimeType(std::string* mime_type) override;
  size_t Read(void* buf, size_t buf_size) override;

 protected:
  ~ProtocolStringJob() override;

 private:
  std::string mime_type_;
  std::string content_;
  size_t pos_ = 0;
};

}  // namespace nu

#endif  // NATIVEUI_PROTOCOL_JOB_H_
