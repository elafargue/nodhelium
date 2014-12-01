#include <node.h>
#include "helium_wrapper.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Helium::Init(exports);
}

NODE_MODULE(helium, InitAll)