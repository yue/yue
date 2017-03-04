#include <node.h>

void Init(v8::Local<v8::Object> exports) {
  fprintf(stderr, "Loaded\n");
}

NODE_MODULE(yue, Init)
