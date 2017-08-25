// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TOOLBAR_H_
#define NATIVEUI_TOOLBAR_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/geometry/size_f.h"
#include "nativeui/types.h"

namespace nu {

class Image;
class View;

class NATIVEUI_EXPORT Toolbar : public base::RefCounted<Toolbar> {
 public:
  // Standard toolbar item identifiers.
  static const char kFlexibleSpaceItemIdentifier[];
  static const char kSpaceItemIdentifier[];

  struct Item {
    Item();
    Item(Item&&);
    ~Item();

    Item& operator=(Item&&);

    std::string label;
    SizeF min_size;
    SizeF max_size;
    scoped_refptr<Image> image;
    scoped_refptr<View> view;
    std::function<void(Toolbar*, const std::string&)> on_click;
    std::vector<std::string> subitems;
  };

  enum class DisplayMode {
    Default,
    IconAndLabel,
    Icon,
    Label,
  };

  explicit Toolbar(const std::string& identifier);

  void SetDefaultItemIdentifiers(const std::vector<std::string>& identifiers);
  void SetAllowedItemIdentifiers(const std::vector<std::string>& identifiers);
  void SetAllowCustomization(bool allow);
  void SetDisplayMode(DisplayMode mode);
  void SetVisible(bool visible);
  bool IsVisible() const;
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
