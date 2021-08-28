// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TEXT_EDIT_H_
#define NATIVEUI_TEXT_EDIT_H_

#include <string>
#include <tuple>

#include "nativeui/scroll.h"

namespace nu {

class NATIVEUI_EXPORT TextEdit : public View {
 public:
  TextEdit();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

  void SetText(const std::string& text);
  std::string GetText() const;

  void Redo();
  bool CanRedo() const;
  void Undo();
  bool CanUndo() const;

  void Cut();
  void Copy();
  void Paste();
  void SelectAll();

  std::tuple<int, int> GetSelectionRange() const;
  void SelectRange(int start, int end);
  std::string GetTextInRange(int start, int end) const;
  void InsertText(const std::string& text);
  void InsertTextAt(const std::string& text, int pos);
  void Delete();
  void DeleteRange(int start, int end);

#if !defined(OS_WIN)
  void SetOverlayScrollbar(bool overlay);
#endif
  void SetScrollbarPolicy(Scroll::Policy h_policy, Scroll::Policy v_policy);

#if defined(OS_MACOSX)
  void SetScrollElasticity(Scroll::Elasticity h, Scroll::Elasticity v);
  std::tuple<Scroll::Elasticity, Scroll::Elasticity> GetScrollElasticity()
      const;
#endif

  RectF GetTextBounds() const;

  // Events.
  Signal<void(TextEdit*)> on_text_change;

  // Delegate methods.
  std::function<bool(TextEdit*)> should_insert_new_line;

 protected:
  ~TextEdit() override;
};

}  // namespace nu

#endif  // NATIVEUI_TEXT_EDIT_H_
