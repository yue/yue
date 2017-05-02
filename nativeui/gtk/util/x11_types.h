// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_UTIL_X11_TYPES_H_
#define NATIVEUI_GTK_UTIL_X11_TYPES_H_

#include <stdint.h>

typedef unsigned long XAtom;
typedef unsigned long XID;
typedef unsigned long VisualID;
typedef struct _XcursorImage XcursorImage;
typedef union _XEvent XEvent;
typedef struct _XImage XImage;
typedef struct _XGC *GC;
typedef struct _XDisplay XDisplay;
typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef XID GLXWindow;
typedef XID GLXDrawable;

extern "C" {
int XFree(void*);
}

#endif  // NATIVEUI_GTK_UTIL_X11_TYPES_H_
