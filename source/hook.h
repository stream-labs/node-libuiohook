#pragma once

/// Node/NAN
#include <node.h>
#include <nan.h>
#include <uv.h>
/// Not node/nan
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>
#include <queue>
#include <inttypes.h>
#ifdef _WIN32
#include <chrono>
#include <processthreadsapi.h>
#else
#include <uiohook.h>
#endif
#include <iostream>
//#include <stdio.h>
//#include <iostream>
//#include <condition_variable>


typedef int16_t key_t;
typedef int16_t modifier_t;
typedef int32_t keyModifier_t;

struct HotKey {
	keyModifier_t keyModifierCombo;
	int8_t state; // 0 = Up, 1 = Down, -1 = Fail
	std::unique_ptr<Nan::Callback> pressedcb, releasedcb;

	#ifdef _WIN32
	int32_t id;
	key_t virtualKey;
	modifier_t modifiers;
	#else

	#endif

	HotKey(keyModifier_t keyCombo) {
		keyModifierCombo = keyCombo;
		state = 0;

		#ifdef _WIN32
		id = 0;
        virtualKey = (keyCombo >> 8) & 0xFF;
		modifiers = keyCombo & 0xFF;
		#endif
	};
	~HotKey() {}
};
static std::map<keyModifier_t, std::shared_ptr<HotKey>> g_hotKeyMap;

int32_t HotKeyThread(void* arg);
keyModifier_t ConvertStringToKeyCombo(std::string keyString);

void JSStartHook(const v8::FunctionCallbackInfo<v8::Value>& args);
void JSStopHook(const v8::FunctionCallbackInfo<v8::Value>& args);
void JSRegisterHotKey(const v8::FunctionCallbackInfo<v8::Value>& args);
void JSUnregisterHotKey(const v8::FunctionCallbackInfo<v8::Value>& args);
void JSUnregisterAllHotKeys(const v8::FunctionCallbackInfo<v8::Value>& args);

class ForeignWorker {
private:
    uv_async_t *async;

    static void AsyncClose(uv_handle_t *handle) {
        ForeignWorker *worker =
            reinterpret_cast<ForeignWorker*>(handle->data);

        worker->Destroy();
    }

    static NAUV_WORK_CB(AsyncCallback) {
        ForeignWorker *worker =
            reinterpret_cast<ForeignWorker*>(async->data);
        worker->Execute();
        uv_close(reinterpret_cast<uv_handle_t*>(async), ForeignWorker::AsyncClose);
    }

protected:
    Nan::Callback *callback;

    v8::Local<v8::Value> Call(int argc = 0, v8::Local<v8::Value> params[] = 0) {
        return callback->Call(argc, params);
    }

public:
    ForeignWorker(Nan::Callback *callback) {
        async = new uv_async_t;

        uv_async_init(
            uv_default_loop()
            , async
            , AsyncCallback
            );

        async->data = this;
        this->callback = callback;
    }

    void Send() {
        uv_async_send(async);
    }

    virtual void Execute() = 0;
    virtual void Destroy() {
        delete this;
    };

    virtual ~ForeignWorker() 
    {
        delete async;
    }
};

class Worker : public ForeignWorker {
public:
    Worker(Nan::Callback *callback)
        : ForeignWorker(callback)
    {
    }

    virtual void Execute() {
        Call(0, 0);
    }

    virtual void Destroy() {
        delete this;
    }
};
