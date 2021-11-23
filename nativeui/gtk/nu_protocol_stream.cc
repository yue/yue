// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/nu_protocol_stream.h"

#include "nativeui/message_loop.h"
#include "nativeui/protocol_job.h"

namespace nu {

struct _NUProtocolStreamPrivate {
  ProtocolJob* protocol_job;
};

G_DEFINE_TYPE_WITH_PRIVATE(NUProtocolStream,
                           nu_protocol_stream,
                           G_TYPE_INPUT_STREAM)

static void nu_protocol_stream_finialize(GObject* stream) {
  NUProtocolStreamPrivate* priv = NU_PROTOCOL_STREAM(stream)->priv;
  // Free the ProtocolJob on the main thread.
  ProtocolJob* protocol_job = priv->protocol_job;
  MessageLoop::PostTask([protocol_job]() {
    protocol_job->Release();
  });

  G_OBJECT_CLASS(nu_protocol_stream_parent_class)->finalize(stream);
}

static gssize nu_protocol_stream_read(GInputStream* stream,
                                      void* buffer, gsize count,
                                      GCancellable*, GError**) {
  NUProtocolStreamPrivate* priv = NU_PROTOCOL_STREAM(stream)->priv;
  return priv->protocol_job->Read(buffer, count);
}

static gboolean nu_protocol_stream_close(GInputStream* stream,
                                         GCancellable*, GError**) {
  return true;
}

static void nu_protocol_stream_class_init(NUProtocolStreamClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = nu_protocol_stream_finialize;

  GInputStreamClass* istream_class = G_INPUT_STREAM_CLASS(klass);
  istream_class->read_fn = nu_protocol_stream_read;
  istream_class->close_fn = nu_protocol_stream_close;
}

static void nu_protocol_stream_init(NUProtocolStream* stream) {
  stream->priv = static_cast<NUProtocolStreamPrivate*>(
      nu_protocol_stream_get_instance_private(stream));
}

GInputStream* nu_protocol_stream_new(ProtocolJob* protocol_job) {
  void* stream = g_object_new(NU_TYPE_PROTOCOL_STREAM, nullptr);
  NUProtocolStreamPrivate* priv = NU_PROTOCOL_STREAM(stream)->priv;
  priv->protocol_job = protocol_job;
  priv->protocol_job->AddRef();
  return G_INPUT_STREAM(stream);
}

}  // namespace nu
