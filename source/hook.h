#pragma once
#include <nan.h>
#include <uv.h>

void StartHotkeyThreadJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void StopHotkeyThreadJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void RegisterHotkeyJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void UnregisterHotkeyJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void UnregisterHotkeysJS(const v8::FunctionCallbackInfo<v8::Value>& args);
