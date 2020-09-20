// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TAB_H_
#define NATIVEUI_TAB_H_

#include <string>
#include <vector>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Tab : public View {
 public:
  Tab();

  // View class name.
  static const char kClassName[];

  void AddPage(const std::string& title, scoped_refptr<View> view);
  void RemovePage(View* view);

  int PageCount() const { return static_cast<int>(pages_.size()); }
  View* PageAt(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= pages_.size())
      return nullptr;
    return pages_[index].get();
  }

  void SelectPageAt(int index);
  int GetSelectedPageIndex() const;
  View* GetSelectedPage() const { return PageAt(GetSelectedPageIndex()); }

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

  // Events.
  Signal<void(Tab*)> on_selected_page_change;

 protected:
  ~Tab() override;

 private:
  NativeView PlatformCreate();
  void PlatformAddPage(const std::string& title, View* view);
  void PlatformRemovePage(int index, View* view);

  std::vector<scoped_refptr<View>> pages_;
};

}  // namespace nu

#endif  // NATIVEUI_TAB_H_
