// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_UTIL_UNDOABLE_TEXT_BUFFER_H_
#define NATIVEUI_GTK_UTIL_UNDOABLE_TEXT_BUFFER_H_

typedef struct _GtkTextBuffer GtkTextBuffer;

namespace nu {

// Attach undo/redo stacks to a text buffer.
void TextBufferMakeUndoable(GtkTextBuffer* buffer);
bool TextBufferIsUndoable(GtkTextBuffer* buffer);

// Manipulate the undo/redo stacks.
void TextBufferUndo(GtkTextBuffer* buffer);
bool TextBufferCanUndo(GtkTextBuffer* buffer);
void TextBufferRedo(GtkTextBuffer* buffer);
bool TextBufferCanRedo(GtkTextBuffer* buffer);

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_UNDOABLE_TEXT_BUFFER_H_
