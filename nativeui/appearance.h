// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_APPEARANCE_H_
#define NATIVEUI_APPEARANCE_H_

#include <memory>

#include "base/memory/weak_ptr.h"
#include "nativeui/signal.h"

namespace nu {

class Appearance;

namespace internal {

// Internal: Interface for platform observers.
class ColorSchemeObserver {
 public:
  static ColorSchemeObserver* Create(Appearance* appearance);
  virtual ~ColorSchemeObserver() {}
};

}  // namespace internal

class NATIVEUI_EXPORT Appearance : public SignalDelegate {
 public:
  ~Appearance() override;

  static Appearance* GetCurrent();

#if defined(OS_WIN)
  void SetDarkModeEnabled(bool enable);
#endif
  bool IsDarkScheme() const;

  base::WeakPtr<Appearance> GetWeakPtr() { return weak_factory_.GetWeakPtr(); }

  // Events.
  Signal<void()> on_color_scheme_change;

 protected:
  Appearance();

  // SignalDelegate:
  void OnConnect(int identifier) override;

 private:
  friend class State;

  enum { kColorSchemeChange };

  std::unique_ptr<internal::ColorSchemeObserver> color_scheme_observer_;

  base::WeakPtrFactory<Appearance> weak_factory_;
};

}  // namespace nu

#endif  // NATIVEUI_APPEARANCE_H_
