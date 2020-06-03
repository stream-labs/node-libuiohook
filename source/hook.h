/******************************************************************************
    Copyright (C) 2016-2020 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#pragma once
#include <nan.h>
#include <uv.h>
#include <iostream>

class ForeignWorker {
	private:
	uv_async_t * async;

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

	virtual ~ForeignWorker() {
		delete async;
	}
};

class Worker : public ForeignWorker {
	public:
	Worker(Nan::Callback *callback)
		: ForeignWorker(callback) {}

	virtual void Execute() {
		Call(0, 0);
	}

	virtual void Destroy() {
		delete this;
	}
};

void StartHotkeyThreadJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void StopHotkeyThreadJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void RegisterHotkeyJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void UnregisterHotkeyJS(const v8::FunctionCallbackInfo<v8::Value>& args);
void UnregisterHotkeysJS(const v8::FunctionCallbackInfo<v8::Value>& args);
