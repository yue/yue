// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_VIEW_H_
#define NATIVEUI_VIEW_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

typedef struct CSSNode *CSSNodeRef;

namespace nu {

// The base class for all kinds of views.
NATIVEUI_EXPORT class View : public base::RefCounted<View> {
 public:
  NativeView view() const { return view_; }

  // Change/Get position and size.
  void SetBounds(const Rect& bounds);
  Rect GetBounds() const;

  // The real pixel bounds that depends on the scale factor.
  void SetPixelBounds(const Rect& bounds);
  Rect GetPixelBounds() const;

  // Get the offset to the parent window.
  Point GetWindowOrigin() const;
  Point GetWindowPixelOrigin() const;

  // Show/Hide the view.
  void SetVisible(bool visible);
  bool IsVisible() const;

  // Set the preferred size of the view, returns whether current view should
  // do a layout.
  bool SetPreferredSize(const Size& size);
  bool SetPixelPreferredSize(const Size& size);
  Size GetPixelPreferredSize() const;

  // A child changed its preferred size, returns true if the child should do a
  // layout.
  virtual bool UpdatePreferredSize();

  // Set CSS style for the node.
  void SetStyle(const std::string& name, const std::string& value);

  // Convert the DIP geometry to pixel geometry.
  int DIPToPixel(int length) const;

  // Get the size the view would like to be.
  Size preferred_size() const { return preferred_size_; }

  // Get parent.
  View* parent() const { return parent_; }

  // Set parent, can only be used internally for now.
  void set_parent(View* parent) { parent_ = parent; }

  // Get the CSS node of the view, can only be used internally.
  CSSNodeRef node() const { return node_; }

 protected:
  View();
  virtual ~View();

  // Called by subclasses to take the ownership of |view|.
  void TakeOverView(NativeView view);

  // Called by sublcasses for setting preferred size.
  bool DoSetPreferredSize(const Size& size);

 private:
  friend class base::RefCounted<View>;

  void PlatformInit();
  void PlatformDestroy();
  void PlatformSetVisible(bool visible);

  // Relationships.
  View* parent_ = nullptr;

  // The native implementation.
  NativeView view_;

  // The preferred size of widget.
  Size preferred_size_;

  // The node recording CSS styles.
  CSSNodeRef node_;
};

}  // namespace nu

#endif  // NATIVEUI_VIEW_H_
