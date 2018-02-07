// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_NU_PROTOCOL_STREAM_H_
#define NATIVEUI_GTK_NU_PROTOCOL_STREAM_H_

#include <gio/gio.h>

// Custom GIO input stream for wrapping ProtocolJob.

namespace nu {

class ProtocolJob;

#define NU_TYPE_PROTOCOL_STREAM         (nu_protocol_stream_get_type())
#define NU_PROTOCOL_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), \
                                         NU_TYPE_PROTOCOL_STREAM, \
                                         NUProtocolStream))
#define NU_PROTOCOL_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), \
                                         NU_TYPE_PROTOCOL_STREAM, \
                                         NUProtocolStreamClass))
#define NU_PROTOCOL_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), \
                                         NU_TYPE_PROTOCOL_STREAM, \
                                         NUProtocolStreamClass))

typedef struct _NUProtocolStream         NUProtocolStream;
typedef struct _NUProtocolStreamClass    NUProtocolStreamClass;
typedef struct _NUProtocolStreamPrivate  NUProtocolStreamPrivate;

struct _NUProtocolStream {
  GInputStream parent_instance;
  NUProtocolStreamPrivate* priv;
};

struct _NUProtocolStreamClass {
  GInputStreamClass parent_class;
};

GType nu_protocol_stream_get_type();
GInputStream* nu_protocol_stream_new(ProtocolJob*);

}  // namespace nu

#endif  // NATIVEUI_GTK_NU_PROTOCOL_STREAM_H_
