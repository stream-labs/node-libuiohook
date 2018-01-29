#include <node.h>
#include "hook.h"

using namespace v8;

void init(Local<Object> exports) {
    /// Functions ///
    NODE_SET_METHOD(exports, "startHook", StartHotkeyThreadJS);
    NODE_SET_METHOD(exports, "stopHook", StopHotkeyThreadJS);
    NODE_SET_METHOD(exports, "registerCallback", RegisterHotkeyJS);
    NODE_SET_METHOD(exports, "unregisterCallback", UnregisterHotkeyJS);
    NODE_SET_METHOD(exports, "unregisterAllCallbacks", UnregisterHotkeysJS);
}

NODE_MODULE(uiohookModule, init)