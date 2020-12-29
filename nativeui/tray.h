// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TRAY_H_
#define NATIVEUI_TRAY_H_

#include <string>

#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class Image;
class Menu;
class RectF;

class NATIVEUI_EXPORT Tray : public base::RefCounted<Tray> {
 public:
  explicit Tray(scoped_refptr<Image> icon);

#if defined(OS_MAC)
  // Text only tray icon.
  explicit Tray(const std::string& title);
#endif

  void Remove();
#if defined(OS_MAC) || defined(OS_WIN)
  RectF GetBounds() const;
#endif
#if defined(OS_MAC) || defined(OS_LINUX)
  void SetTitle(const std::string& title);
#endif
  void SetImage(scoped_refptr<Image> icon);
#if defined(OS_MAC)
  void SetPressedImage(scoped_refptr<Image> icon);
#endif
  void SetMenu(scoped_refptr<Menu> menu);
  Menu* GetMenu() const { return menu_.get(); }

  NativeTray GetNative() const { return tray_; }

  Signal<void(Tray*)> on_click;

 protected:
  virtual ~Tray();

 private:
  friend class base::RefCounted<Tray>;

#if defined(OS_LINUX)
  // On Linux we have to save images to filesystem to use them.
  std::string WriteImage(Image* icon);

  // The ID of the app indicator.
  std::string id_;

  // Dir for storing the image.
  base::ScopedTempDir temp_dir_;

  // Generated ID used for image file names.
  int icon_change_count_ = 0;
#endif

  scoped_refptr<Menu> menu_;

  NativeTray tray_ = nullptr;
};

}  // namespace nu

#endif  // NATIVEUI_TRAY_H_
