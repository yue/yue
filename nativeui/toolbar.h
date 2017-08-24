// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TOOLBAR_H_
#define NATIVEUI_TOOLBAR_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class Image;
class View;

class NATIVEUI_EXPORT Toolbar : public base::RefCounted<Toolbar> {
 public:
  explicit Toolbar(const std::string& identifier);

  struct Item {
    Item();
    Item(const Item& item);
    ~Item();

    std::string label;
    scoped_refptr<Image> image;
    scoped_refptr<View> view;
    std::function<void(Toolbar*, const std::string&)> on_click;
    std::vector<std::string> subitems;
  };

  void SetDefaultItemIdentifiers(const std::vector<std::string>& identifiers);
  void SetAllowedItemIdentifiers(const std::vector<std::string>& identifiers);
  std::string GetIdentifier() const;

  NativeToolbar GetNative() const { return toolbar_; }

  // Delegate methods.
  std::function<Item(Toolbar*, const std::string&)> get_item;

 protected:
  virtual ~Toolbar();

 private:
  friend class base::RefCounted<Toolbar>;

  NativeToolbar toolbar_;
};

}  // namespace nu

#endif  // NATIVEUI_TOOLBAR_H_
