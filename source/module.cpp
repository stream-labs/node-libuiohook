#include <node.h>
#include "hook.h"

using namespace v8;

void init(Local<Object> exports) {
    /// Functions ///
    NODE_SET_METHOD(exports, "startHook", JSStartHook);
    NODE_SET_METHOD(exports, "stopHook", JSStopHook);
    NODE_SET_METHOD(exports, "registerCallback", JSRegisterHotKey);
    NODE_SET_METHOD(exports, "unregisterCallback", JSUnregisterHotKey);
    NODE_SET_METHOD(exports, "unregisterAllCallbacks", JSUnregisterAllHotKeys);
}

NODE_MODULE(uiohookModule, init)