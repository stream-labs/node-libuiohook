#include "hook.h"

static std::mutex g_hotKeyMutex;
static std::thread g_hotKeyThread;
#ifdef _WIN32
#define WM_HOTKEY_REGISTER WM_USER + 1
#define WM_HOTKEY_UNREGISTER WM_USER + 2

static int32_t m_nextFreeId = 1;
static std::queue<int32_t> m_freeIdQueue;
#else
#define UIOHOOK_ERROR_THREAD_CREATE				0x10

static pthread_t hook_thread;
static pthread_mutex_t hook_running_mutex;
static pthread_mutex_t hook_control_mutex;
static pthread_cond_t hook_control_cond;
#endif


static std::map<std::string, key_t> g_KeyMap = {
	#ifdef _WIN32
	// Mouse
	std::make_pair("LeftMouseButton", VK_LBUTTON), std::make_pair("RightMouseButton", VK_RBUTTON),
	std::make_pair("MiddleMouseButton", VK_MBUTTON),
	std::make_pair("X1MouseButton", VK_XBUTTON1), std::make_pair("X2MouseButton", VK_XBUTTON2),
	// Keyboard
	std::make_pair("Backspace", VK_BACK),
	std::make_pair("Tab", VK_TAB),
	std::make_pair("Clear", VK_CLEAR),
	std::make_pair("Enter", VK_RETURN),
	std::make_pair("Shift", VK_SHIFT), std::make_pair("ShiftLeft", VK_LSHIFT), std::make_pair("ShiftRight", VK_RSHIFT),
	std::make_pair("Control", VK_CONTROL), std::make_pair("ControlLeft", VK_LCONTROL), std::make_pair("ControlRight", VK_RCONTROL),
	std::make_pair("Command", VK_CONTROL), std::make_pair("LeftCommand", VK_LCONTROL), std::make_pair("RightCommand", VK_RCONTROL),
	std::make_pair("CommandOrControl", VK_CONTROL), std::make_pair("LeftCommandOrControl", VK_LCONTROL), std::make_pair("RightCommandOrControl", VK_RCONTROL),
	std::make_pair("Alt", VK_MENU), std::make_pair("AltLeft", VK_LMENU), std::make_pair("AltRight", VK_RMENU),
	std::make_pair("Menu", VK_MENU), std::make_pair("LeftMenu", VK_LMENU), std::make_pair("RightMenu", VK_RMENU),
	std::make_pair("OSLeft", VK_LWIN), std::make_pair("OSRight", VK_RWIN),
	std::make_pair("Pause", VK_PAUSE),
	std::make_pair("Capital", VK_CAPITAL), std::make_pair("CapsLock", VK_CAPITAL),
	std::make_pair("NumLock", VK_NUMLOCK),
	std::make_pair("ScrollLock", VK_SCROLL),
	std::make_pair("Escape", VK_ESCAPE),
	std::make_pair("Space", VK_SPACE),
	std::make_pair("PageUp", VK_PRIOR), std::make_pair("PageDown", VK_NEXT),
	std::make_pair("Home", VK_HOME), std::make_pair("End", VK_END),
	std::make_pair("Left", VK_LEFT), std::make_pair("Right", VK_RIGHT),
	std::make_pair("Up", VK_UP), std::make_pair("Down", VK_DOWN),
	std::make_pair("Select", VK_SELECT),
	std::make_pair("Print", VK_PRINT),
	std::make_pair("Execute", VK_EXECUTE),
	std::make_pair("Snapshot", VK_SNAPSHOT),
	std::make_pair("PrintScreen", VK_SNAPSHOT),
	std::make_pair("Insert", VK_INSERT), std::make_pair("Delete", VK_DELETE),
	std::make_pair("Help", VK_HELP),
	std::make_pair("Apps", VK_APPS),
	std::make_pair("Sleep", VK_SLEEP),
	/// Function
	std::make_pair("F1", VK_F1), std::make_pair("F2", VK_F2), std::make_pair("F3", VK_F3),
	std::make_pair("F4", VK_F4), std::make_pair("F5", VK_F5), std::make_pair("F6", VK_F6),
	std::make_pair("F7", VK_F7), std::make_pair("F8", VK_F8), std::make_pair("F9", VK_F9),
	std::make_pair("F10", VK_F10), std::make_pair("F11", VK_F11), std::make_pair("F12", VK_F12),
	std::make_pair("F13", VK_F13), std::make_pair("F14", VK_F14), std::make_pair("F15", VK_F15),
	std::make_pair("F16", VK_F16), std::make_pair("F17", VK_F17), std::make_pair("F18", VK_F18),
	std::make_pair("F19", VK_F19), std::make_pair("F20", VK_F20), std::make_pair("F21", VK_F21),
	std::make_pair("F22", VK_F22), std::make_pair("F23", VK_F23), std::make_pair("F24", VK_F24),
	/// Numeric
	std::make_pair("Digit0", 0x30), std::make_pair("Digit1", 0x31), std::make_pair("Digit2", 0x32),
	std::make_pair("Digit3", 0x33), std::make_pair("Digit4", 0x34), std::make_pair("Digit5", 0x35),
	std::make_pair("Digit6", 0x36), std::make_pair("Digit7", 0x37), std::make_pair("Digit8", 0x38),
	std::make_pair("Digit9", 0x39),
	/// Letters
	std::make_pair("KeyA", 0x41), std::make_pair("KeyB", 0x42), std::make_pair("KeyC", 0x43),
	std::make_pair("KeyD", 0x44), std::make_pair("KeyE", 0x45), std::make_pair("KeyF", 0x46),
	std::make_pair("KeyG", 0x47), std::make_pair("KeyH", 0x48), std::make_pair("KeyI", 0x49),
	std::make_pair("KeyJ", 0x4A), std::make_pair("KeyK", 0x4B), std::make_pair("KeyL", 0x4C),
	std::make_pair("KeyM", 0x4D), std::make_pair("KeyN", 0x4E), std::make_pair("KeyO", 0x4F),
	std::make_pair("KeyP", 0x50), std::make_pair("KeyQ", 0x51), std::make_pair("KeyR", 0x52),
	std::make_pair("KeyS", 0x53), std::make_pair("KeyT", 0x54), std::make_pair("KeyU", 0x55),
	std::make_pair("KeyV", 0x56), std::make_pair("KeyW", 0x57), std::make_pair("KeyX", 0x58),
	std::make_pair("KeyY", 0x59), std::make_pair("KeyZ", 0x5A),
	/// Numeric Pad
	std::make_pair("Numpad0", VK_NUMPAD0), std::make_pair("Numpad1", VK_NUMPAD1),
	std::make_pair("Numpad2", VK_NUMPAD2), std::make_pair("Numpad3", VK_NUMPAD3),
	std::make_pair("Numpad4", VK_NUMPAD4), std::make_pair("Numpad5", VK_NUMPAD5),
	std::make_pair("Numpad6", VK_NUMPAD6), std::make_pair("Numpad7", VK_NUMPAD7),
	std::make_pair("Numpad8", VK_NUMPAD8), std::make_pair("Numpad9", VK_NUMPAD9),
	std::make_pair("NumpadMultiply", VK_MULTIPLY), std::make_pair("NumpadDivide", VK_DIVIDE),
	std::make_pair("NumpadAdd", VK_ADD), std::make_pair("NumpadSubtract", VK_SUBTRACT),
	std::make_pair("Separator", VK_SEPARATOR), std::make_pair("NumpadDecimal", VK_DECIMAL),
	std::make_pair("NumLock", VK_NUMLOCK), std::make_pair("NumpadEnter", VK_RETURN),

	/// OEM Keys
	std::make_pair("Semicolon", VK_OEM_1), std::make_pair("Equal", VK_OEM_PLUS),
	std::make_pair("Comma", VK_OEM_COMMA), std::make_pair("Minus", VK_OEM_MINUS),
	std::make_pair("Period", VK_OEM_PERIOD), std::make_pair("Slash", VK_OEM_2),
	std::make_pair("Backquote", VK_OEM_3), std::make_pair("BracketLeft", VK_OEM_4),
	std::make_pair("Backslash", VK_OEM_5), std::make_pair("BracketRight", VK_OEM_6),
	std::make_pair("Quote", VK_OEM_7),
	// Arrows
	std::make_pair("ArrowUp", VK_UP), std::make_pair("ArrowLeft", VK_LEFT),
	std::make_pair("ArrowRight", VK_RIGHT), std::make_pair("ArrowDown", VK_DOWN),

	#else
	std::make_pair("Escape", VC_ESCAPE),
	std::make_pair("F1", VC_F1),
	std::make_pair("F2", VC_F2),
	std::make_pair("F3", VC_F3),
	std::make_pair("F4", VC_F4),
	std::make_pair("F5", VC_F5),
	std::make_pair("F6", VC_F6),
	std::make_pair("F7", VC_F7),
	std::make_pair("F8", VC_F8),
	std::make_pair("F9", VC_F9),
	std::make_pair("F10", VC_F10),
	std::make_pair("F11", VC_F11),
	std::make_pair("F12", VC_F12),
	std::make_pair("F13", VC_F13),
	std::make_pair("F14", VC_F14),
	std::make_pair("F15", VC_F15),
	std::make_pair("F16", VC_F16),
	std::make_pair("F17", VC_F17),
	std::make_pair("F18", VC_F18),
	std::make_pair("F19", VC_F19),
	std::make_pair("F20", VC_F20),
	std::make_pair("F21", VC_F21),
	std::make_pair("F22", VC_F22),
	std::make_pair("F23", VC_F23),
	std::make_pair("F24", VC_F24),
	std::make_pair("1", VC_1),
	std::make_pair("2", VC_2),
	std::make_pair("3", VC_3),
	std::make_pair("4", VC_4),
	std::make_pair("5", VC_5),
	std::make_pair("6", VC_6),
	std::make_pair("7", VC_7),
	std::make_pair("8", VC_8),
	std::make_pair("9", VC_9),
	std::make_pair("0", VC_0),
	std::make_pair("Backspace", VC_BACKSPACE),
	std::make_pair("Tab", VC_TAB),
	std::make_pair("A", VC_A),
	std::make_pair("B", VC_B),
	std::make_pair("C", VC_C),
	std::make_pair("D", VC_D),
	std::make_pair("E", VC_E),
	std::make_pair("F", VC_F),
	std::make_pair("G", VC_G),
	std::make_pair("H", VC_H),
	std::make_pair("I", VC_I),
	std::make_pair("J", VC_J),
	std::make_pair("K", VC_K),
	std::make_pair("L", VC_L),
	std::make_pair("M", VC_M),
	std::make_pair("N", VC_N),
	std::make_pair("O", VC_O),
	std::make_pair("P", VC_P),
	std::make_pair("Q", VC_Q),
	std::make_pair("R", VC_R),
	std::make_pair("S", VC_S),
	std::make_pair("T", VC_T),
	std::make_pair("U", VC_U),
	std::make_pair("V", VC_V),
	std::make_pair("W", VC_W),
	std::make_pair("X", VC_X),
	std::make_pair("Y", VC_Y),
	std::make_pair("Z", VC_Z),
	std::make_pair("Control", 29),
	std::make_pair("CommandOrControl", 29),
	std::make_pair("Command", 29),
	std::make_pair("Alt", 56),
	std::make_pair("Shift", 42),
	#endif
};

static std::map<std::string, modifier_t> g_ModifierMap = {
	#ifdef _WIN32
	std::make_pair("Alt", MOD_ALT),
	std::make_pair("Ctrl", MOD_CONTROL),
	std::make_pair("CommandOrControl", MOD_CONTROL),
	std::make_pair("Shift", MOD_SHIFT),
	std::make_pair("Super", MOD_WIN),
	//std::make_pair("Win", MOD_WIN),
	#else
	std::make_pair("Alt", MASK_ALT),
	std::make_pair("Control", MASK_CTRL),
	std::make_pair("CommandOrControl", MASK_CTRL),
	std::make_pair("Shift", MASK_SHIFT),
	#endif
};

int32_t HotKeyThread(void* arg) {
	namespace schr = std::chrono;
	using scrhrc = std::chrono::high_resolution_clock;

	// Temporarily prevent execution until main is ready.
	{
		std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
	}

	#ifdef _WIN32
	MSG msg; BOOL ret; bool do_exit = false;
	while (!do_exit) {
		// Time Wait for new messages.
		if ((ret = PeekMessage(&msg, (HWND)-1, 0, 0, PM_NOREMOVE)) == 0) {
			auto waitBegin = scrhrc::now();
			auto waitMax = schr::milliseconds(16);
			auto waitTotal = schr::milliseconds(0);
			do {
				waitTotal = schr::duration_cast<schr::milliseconds>(scrhrc::now() - waitBegin);
				ret = PeekMessage(&msg, (HWND)-1, 0, 0, PM_NOREMOVE);
				if (ret != 0)
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			} while (waitTotal.count() < 16 && ret == 0);
		}

		if (ret != 0) {
			// We have a message to process.
			ret = GetMessage(&msg, (HWND)-1, 0, 0);
			if (ret == -1)
				break;

			switch (msg.message) {
				case WM_CREATE:
					break;
				case WM_DESTROY:
					do_exit = true;
					break;
				case WM_HOTKEY:
					{
						std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
						keyModifier_t code = msg.lParam;
						if (g_hotKeyMap.count(code) > 0) {
							std::shared_ptr<HotKey> hk = g_hotKeyMap.find(code)->second;
							if (hk->state == 0) {
								hk->state = 1;
								if (hk->pressedcb != nullptr) {
									Worker *worker = new Worker(hk->pressedcb.get());
									worker->Send();
								}
							}
						}
					}
					break;
				case WM_HOTKEY_REGISTER:
					{
						std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
						keyModifier_t code = msg.lParam;
						std::shared_ptr<HotKey> hk = g_hotKeyMap.find(code)->second;

						if (m_freeIdQueue.size() > 0) {
							hk->id = m_freeIdQueue.front();
							m_freeIdQueue.pop();
						} else {
							hk->id = m_nextFreeId++;
						}

						BOOL hret = RegisterHotKey(0, hk->id, hk->modifiers, hk->virtualKey);
						if (hret == 0) {
							DWORD errorMessageID = ::GetLastError();
							if(errorMessageID != 0) {
								LPSTR messageBuffer = nullptr;
								size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
															NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

								std::string message(messageBuffer, size);

								//Free the buffer.
								LocalFree(messageBuffer);

								std::cout << "Error : " << msg.lParam << " : " << message.c_str() << std::endl;
							}
							// Error Handling?
							hk->state = -1;
						} else {
							hk->state = 0;
						}
					}
					break;
				case WM_HOTKEY_UNREGISTER:
					{
						std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
						BOOL hret = UnregisterHotKey(0, msg.lParam);
						if (hret == 0) {
							// error logging?
							DWORD errorMessageID = ::GetLastError();
							if(errorMessageID != 0) {
								LPSTR messageBuffer = nullptr;
								size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
															NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

								std::string message(messageBuffer, size);

								//Free the buffer.
								LocalFree(messageBuffer);

								std::cout << "Error : " << msg.lParam << " : " << message.c_str() << std::endl;
							}
						}
						m_freeIdQueue.push(msg.lParam);
					}
					break;
				default:
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					break;
			}
		} else {
			// We don't so just iterate over all hotkeys and set the correct keyup stuff.
			std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
			for (auto kv : g_hotKeyMap) {
				if (kv.second->state == 1) {
					if (GetAsyncKeyState(kv.second->virtualKey) == 0) {
						kv.second->state = 0;
						if (kv.second->releasedcb != nullptr) {
							Worker *worker = new Worker(kv.second->releasedcb.get());
							worker->Send();
						}
					}
				}
			}
		}
	}
	#else

	#endif
	return 0;
}

template < class ContainerT >
void tokenize(const std::string& str, ContainerT& tokens,
	const std::string& delimiters = " ", bool trimEmpty = false) {
	std::string::size_type pos, lastPos = 0, length = str.length();

	using value_type = typename ContainerT::value_type;
	using size_type = typename ContainerT::size_type;

	while (lastPos < length + 1) {
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == std::string::npos) {
			pos = length;
		}

		if (pos != lastPos || !trimEmpty)
			tokens.push_back(value_type(str.data() + lastPos,
			(size_type)pos - lastPos));

		lastPos = pos + 1;
	}
}

template<typename Out> void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

void JSStartHook(const v8::FunctionCallbackInfo<v8::Value>& args) {
	g_hotKeyMutex.lock();
	g_hotKeyMutex.unlock();

	#ifdef _WIN32
	{
		std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
		g_hotKeyThread = std::thread(HotKeyThread, nullptr);
		PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
			WM_CREATE, 0, 0);
		for (auto kv : g_hotKeyMap) {
			PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
				WM_HOTKEY_REGISTER, 0, kv.second->keyModifierCombo);
		}
	}
	#else

	#endif
}

void JSStopHook(const v8::FunctionCallbackInfo<v8::Value>& args) {
	#ifdef _WIN32
	if (g_hotKeyThread.joinable()) {
		{
			std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
			for (auto kv : g_hotKeyMap) {
				PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
					WM_HOTKEY_UNREGISTER, 0, kv.second->id);
			}

			PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
				WM_DESTROY, 0, 0);
		}
		g_hotKeyThread.join();
	}
	#else

	#endif
}

keyModifier_t ConvertStringToKeyCombo(std::string keyString, v8::Local<v8::Object> modifiers) {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();

	key_t keyCode = 0;
	modifier_t modifierCode = 0;

	if (g_KeyMap.count(keyString))
		keyCode = g_KeyMap.find(keyString)->second;

	bool alt = modifiers->Get(v8::String::NewFromUtf8(isolate, "alt"))->ToBoolean(isolate)->Value();
	bool ctrl = modifiers->Get(v8::String::NewFromUtf8(isolate, "ctrl"))->ToBoolean(isolate)->Value();
	bool shift = modifiers->Get(v8::String::NewFromUtf8(isolate, "shift"))->ToBoolean(isolate)->Value();
	bool meta = modifiers->Get(v8::String::NewFromUtf8(isolate, "meta"))->ToBoolean(isolate)->Value();

	if(alt){
		modifierCode |= MOD_ALT;}

	if(ctrl){
		modifierCode |= MOD_CONTROL;}

	if(shift){
		modifierCode |= MOD_SHIFT;}

	if(meta){
		modifierCode |= MOD_WIN;}

	return (keyCode << 16) | (modifierCode);
}

void JSRegisterHotKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	v8::Local<v8::Object> hotkey = v8::Local<v8::Object>::Cast(args[0]);

	v8::String::Utf8Value param0(hotkey->Get(v8::String::NewFromUtf8(isolate, "key")));
	std::string js_keyCode = std::string(*param0);

	v8::Local<v8::Object> modifiers = v8::Local<v8::Object>::Cast(hotkey->Get(v8::String::NewFromUtf8(isolate, "modifiers")));

	keyModifier_t code = ConvertStringToKeyCombo(js_keyCode, modifiers);

	v8::Local<v8::Function> js_function = hotkey->Get(v8::String::NewFromUtf8(isolate, "callback")).As<v8::Function>();

	v8::String::Utf8Value param1(hotkey->Get(v8::String::NewFromUtf8(isolate, "eventType")));
	std::string eventString = std::string(*param1);

	// Create HotKey
	std::shared_ptr<HotKey> hk;
	std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
	bool isKeyUp = false;
	if (g_hotKeyMap.count(code) > 0) {
		hk = g_hotKeyMap.find(code)->second;
	} else {
		hk = std::make_shared<HotKey>(code);
	}

	if (eventString == "registerKeydown") {
		if (hk->pressedcb) {
			args.GetReturnValue().Set(v8::Boolean::New(isolate, false));
			return;
		}
		hk->pressedcb = std::make_unique<Nan::Callback>(js_function);
	} else if (eventString == "registerKeyup") {
		if (hk->releasedcb) {
			args.GetReturnValue().Set(v8::Boolean::New(isolate, false));
			return;
		}
		hk->releasedcb = std::make_unique<Nan::Callback>(js_function);
		isKeyUp = true;
	} else {
		args.GetReturnValue().Set(v8::Boolean::New(isolate, false));
		return;
	}

	g_hotKeyMap.emplace(hk->keyModifierCombo, hk);
	#ifdef _WIN32
	if (hk->id == 0) {
		hk->virtualKey = (hk->keyModifierCombo >> 16) & 0xFF;
		hk->modifiers = hk->keyModifierCombo & 0xFF;
		if (!isKeyUp)
			PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
				WM_HOTKEY_REGISTER, isKeyUp ? 1 : 0, code);
	}
	#else
	#endif

	args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
	return;
}

void JSUnregisterHotKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	v8::Local<v8::Object> hotkey = v8::Local<v8::Object>::Cast(args[0]);

	v8::String::Utf8Value param0(hotkey->Get(v8::String::NewFromUtf8(isolate, "key")));
	std::string js_keyCode = std::string(*param0);

	v8::Local<v8::Object> modifiers = v8::Local<v8::Object>::Cast(hotkey->Get(v8::String::NewFromUtf8(isolate, "modifiers")));

	keyModifier_t code = ConvertStringToKeyCombo(js_keyCode, modifiers);

	v8::String::Utf8Value param1(hotkey->Get(v8::String::NewFromUtf8(isolate, "eventType")));
	std::string eventString = std::string(*param1);

	std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
	if (g_hotKeyMap.count(code) == 0) {
		args.GetReturnValue().Set(v8::Boolean::New(isolate, false));
		return;
	}

	std::shared_ptr<HotKey> hk = g_hotKeyMap.find(code)->second;
	bool isKeyUp = false;

	if (eventString.compare("registerKeydown") == 0) {
		if (hk->pressedcb)
			hk->pressedcb = nullptr;
	} else if (eventString.compare("registerKeyup") == 0) {
		if (hk->releasedcb)
			hk->releasedcb = nullptr;
		isKeyUp = true;
	} else {
		args.GetReturnValue().Set(v8::Boolean::New(isolate, false));
		return;
	}

	if (hk->pressedcb == nullptr && hk->releasedcb == nullptr) {
		#ifdef _WIN32
		{
			if (!isKeyUp)
				PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
					WM_HOTKEY_UNREGISTER, isKeyUp ? 1 : 0, hk->id);
		}
		#else

		#endif
		g_hotKeyMap.erase(hk->keyModifierCombo);
	}

	args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
	return;
}

void JSUnregisterAllHotKeys(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::unique_lock<std::mutex> ulock(g_hotKeyMutex);
	if (g_hotKeyThread.joinable()) {
		for (auto kv : g_hotKeyMap) {
			#ifdef _WIN32
			PostThreadMessage(GetThreadId((HANDLE)g_hotKeyThread.native_handle()),
				WM_HOTKEY_UNREGISTER, 0, kv.second->id);
			#endif
		}
	}
	g_hotKeyMap.clear();
}

//
//
//
//std::vector<std::pair<std::string, int>> keyCodesArray;
//
//std::vector<Action*> pressedKeyEventCallbacks;
//std::vector<Action*> releasedKeyEventCallbacks;
//
//std::vector<std::pair<int64_t, bool>> modifiers;
//
//int64_t currentModifierCode = 0;
//int64_t currentModifierState = EVENT_KEY_RELEASED;
//
//void dispatch_procB(uiohook_event * const event) {
//	switch (event->type) {
//		case EVENT_HOOK_ENABLED:
//			// Lock the running mutex so we know if the hook is enabled.
//			#ifdef _WIN32
//			WaitForSingleObject(hook_running_mutex, INFINITE);
//			#else
//			pthread_mutex_lock(&hook_running_mutex);
//			#endif
//
//
//			#ifdef _WIN32
//			// Signal the control event.
//			SetEvent(hook_control_cond);
//			#else
//			// Unlock the control mutex so hook_enable() can continue.
//			pthread_cond_signal(&hook_control_cond);
//			pthread_mutex_unlock(&hook_control_mutex);
//			#endif
//			break;
//
//		case EVENT_HOOK_DISABLED:
//			// Lock the control mutex until we exit.
//			#ifdef _WIN32
//			WaitForSingleObject(hook_control_mutex, INFINITE);
//			#else
//			pthread_mutex_lock(&hook_control_mutex);
//			#endif
//
//			// Unlock the running mutex so we know if the hook is disabled.
//			#ifdef _WIN32
//			ReleaseMutex(hook_running_mutex);
//			ResetEvent(hook_control_cond);
//			#else
//			#if defined(__APPLE__) && defined(__MACH__)
//			// Stop the main runloop so that this program ends.
//			CFRunLoopStop(CFRunLoopGetMain());
//			#endif
//
//			pthread_mutex_unlock(&hook_running_mutex);
//			#endif
//			break;
//
//		case EVENT_KEY_PRESSED:
//			// std::cout << "key code " << event->data.keyboard.keycode << std::endl;
//			for (int i = 0; i < pressedKeyEventCallbacks.size(); i++) {
//				if (//If the associated event is an EVENT_KEY_PRESSED type
//					pressedKeyEventCallbacks.at(i)->m_event == EVENT_KEY_PRESSED &&
//					//If the current key pressed is associated with an element in the vector
//					event->data.keyboard.keycode == pressedKeyEventCallbacks.at(i)->m_codeEvent.key.code &&
//					//If the key is not already pressed
//					pressedKeyEventCallbacks.at(i)->m_currentState != EVENT_KEY_PRESSED) {
//
//					bool a = pressedKeyEventCallbacks.at(i)->m_codeEvent.modifier.code != -1;
//					bool b = currentModifierState == EVENT_KEY_PRESSED;
//					bool c = pressedKeyEventCallbacks.at(i)->m_codeEvent.modifier.code == currentModifierCode;
//
//					int code = pressedKeyEventCallbacks.at(i)->m_codeEvent.modifier.code;
//
//					if ((!a && !b) || (a && b && c)) {
//						Worker *worker = new Worker(pressedKeyEventCallbacks.at(i)->m_js_callBack);
//						worker->Send();
//
//						pressedKeyEventCallbacks.at(i)->m_currentState = EVENT_KEY_PRESSED;
//						break;
//					}
//				}
//			}
//
//			if (event->data.keyboard.keycode == 29 ||
//				event->data.keyboard.keycode == 56 ||
//				event->data.keyboard.keycode == 42) {
//				currentModifierCode = event->data.keyboard.keycode;
//				currentModifierState = EVENT_KEY_PRESSED;
//			}
//			break;
//		case EVENT_KEY_RELEASED:
//			for (int i = 0; i < releasedKeyEventCallbacks.size(); i++) {
//				if (//If the associated event is an EVENT_KEY_RELEASED type
//					releasedKeyEventCallbacks.at(i)->m_event == EVENT_KEY_RELEASED &&
//					//If the current key pressed is associated with an element in the vector
//					event->data.keyboard.keycode == releasedKeyEventCallbacks.at(i)->m_codeEvent.key.code) {
//
//					bool a = pressedKeyEventCallbacks.at(i)->m_codeEvent.modifier.code != -1;
//					bool b = currentModifierState == EVENT_KEY_PRESSED;
//					bool c = pressedKeyEventCallbacks.at(i)->m_codeEvent.modifier.code == currentModifierCode;
//
//					if ((!a && !b) || (a && b && c)) {
//						Worker *worker = new Worker(releasedKeyEventCallbacks.at(i)->m_js_callBack);
//						worker->Send();
//
//						break;
//					}
//				}
//			}
//
//			for (int i = 0; i < pressedKeyEventCallbacks.size(); i++) {
//				if (//If the associated event is an EVENT_KEY_PRESSED type
//					pressedKeyEventCallbacks.at(i)->m_event == EVENT_KEY_PRESSED &&
//					//If the key is already pressed
//					pressedKeyEventCallbacks.at(i)->m_currentState == EVENT_KEY_PRESSED) {
//					pressedKeyEventCallbacks.at(i)->m_currentState = EVENT_KEY_RELEASED;
//					break;
//				}
//			}
//			if (event->data.keyboard.keycode == 29 ||
//				event->data.keyboard.keycode == 56 ||
//				event->data.keyboard.keycode == 42) {
//				currentModifierState = EVENT_KEY_RELEASED;
//			}
//			break;
//		case EVENT_KEY_TYPED:
//		case EVENT_MOUSE_PRESSED:
//		case EVENT_MOUSE_RELEASED:
//		case EVENT_MOUSE_CLICKED:
//		case EVENT_MOUSE_MOVED:
//		case EVENT_MOUSE_DRAGGED:
//		case EVENT_MOUSE_WHEEL:
//		default:
//			break;
//	}
//}
//
//#ifdef _WIN32
//DWORD WINAPI hook_thread_proc(LPVOID arg) {
//	#else
//void *hook_thread_proc(void *arg) {
//	#endif
//	// Set the hook status.
//	int status = hook_run();
//	if (status != UIOHOOK_SUCCESS) {
//		#ifdef _WIN32
//		*(DWORD *)arg = status;
//		#else
//		*(int *)arg = status;
//		#endif
//	}
//
//	// Make sure we signal that we have passed any exception throwing code for
//	// the waiting hook_enable().
//	#ifdef _WIN32
//	SetEvent(hook_control_cond);
//
//	return status;
//	#else
//	// Make sure we signal that we have passed any exception throwing code for
//	// the waiting hook_enable().
//	pthread_cond_signal(&hook_control_cond);
//	pthread_mutex_unlock(&hook_control_mutex);
//
//	return arg;
//	#endif
//}
//
//int hook_enable() {
//	// Lock the thread control mutex.  This will be unlocked when the
//	// thread has finished starting, or when it has fully stopped.
//	#ifdef _WIN32
//	WaitForSingleObject(hook_control_mutex, INFINITE);
//	#else
//	pthread_mutex_lock(&hook_control_mutex);
//	#endif
//
//	// Set the initial status.
//	int status = UIOHOOK_FAILURE;
//
//	#ifndef _WIN32
//	// Create the thread attribute.
//	pthread_attr_t hook_thread_attr;
//	pthread_attr_init(&hook_thread_attr);
//
//	// Get the policy and priority for the thread attr.
//	int policy;
//	pthread_attr_getschedpolicy(&hook_thread_attr, &policy);
//	int priority = sched_get_priority_max(policy);
//	#endif
//
//	#if defined(_WIN32)
//	DWORD hook_thread_id;
//	DWORD *hook_thread_status = (DWORD *)malloc(sizeof(DWORD));
//	hook_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)hook_thread_proc, hook_thread_status, 0, &hook_thread_id);
//	if (hook_thread != INVALID_HANDLE_VALUE) {
//		#else
//	int *hook_thread_status = malloc(sizeof(int));
//	if (pthread_create(&hook_thread, &hook_thread_attr, hook_thread_proc, hook_thread_status) == 0) {
//		#endif
//		#if defined(_WIN32)
//		// Attempt to set the thread priority to time critical.
//		if (SetThreadPriority(hook_thread, THREAD_PRIORITY_TIME_CRITICAL) == 0) {
//		}
//		#elif (defined(__APPLE__) && defined(__MACH__)) || _POSIX_C_SOURCE >= 200112L
//		// Some POSIX revisions do not support pthread_setschedprio so we will 
//		// use pthread_setschedparam instead.
//		struct sched_param param = { .sched_priority = priority };
//		if (pthread_setschedparam(hook_thread, SCHED_OTHER, &param) != 0) {
//		}
//		#else
//		// Raise the thread priority using glibc pthread_setschedprio.
//		if (pthread_setschedprio(hook_thread, priority) != 0) {
//		}
//		#endif
//
//
//		// Wait for the thread to indicate that it has passed the 
//		// initialization portion by blocking until either a EVENT_HOOK_ENABLED 
//		// event is received or the thread terminates.
//		// NOTE This unlocks the hook_control_mutex while we wait.
//		#ifdef _WIN32
//		WaitForSingleObject(hook_control_cond, INFINITE);
//		#else
//		pthread_cond_wait(&hook_control_cond, &hook_control_mutex);
//		#endif
//
//		#ifdef _WIN32
//		if (WaitForSingleObject(hook_running_mutex, 0) != WAIT_TIMEOUT) {
//			#else
//		if (pthread_mutex_trylock(&hook_running_mutex) == 0) {
//			#endif
//			// Lock Successful; The hook is not running but the hook_control_cond 
//			// was signaled!  This indicates that there was a startup problem!
//
//			// Get the status back from the thread.
//			#ifdef _WIN32
//			WaitForSingleObject(hook_thread, INFINITE);
//			GetExitCodeThread(hook_thread, hook_thread_status);
//			#else
//			pthread_join(hook_thread, (void **)&hook_thread_status);
//			status = *hook_thread_status;
//			#endif
//		} else {
//			// Lock Failure; The hook is currently running and wait was signaled
//			// indicating that we have passed all possible start checks.  We can 
//			// always assume a successful startup at this point.
//			status = UIOHOOK_SUCCESS;
//		}
//
//		free(hook_thread_status);
//
//		} else {
//		status = UIOHOOK_ERROR_THREAD_CREATE;
//		}
//
//	// Make sure the control mutex is unlocked.
//	#ifdef _WIN32
//	ReleaseMutex(hook_control_mutex);
//	#else
//	pthread_mutex_unlock(&hook_control_mutex);
//	#endif
//
//	return status;
//	}
//
//bool logger_proc(unsigned int level, const char *format, ...) {
//	bool status = false;
//
//	return status;
//}
//
//void hook::start(const v8::FunctionCallbackInfo<v8::Value>& args) {
//	#ifdef _WIN32
//	{
//		std::unique_lock<std::mutex> ulock(m_hotkeyMutex);
//		m_hotkeyFlag = true;
//		m_hotkeyThread = std::thread(hotkeyThreadMain);
//		m_hotkeyCV.wait(ulock, [] {return !m_hotkeyFlag; });
//	}
//	// Automatically re-register hotkeys
//	for (auto hk : pressedKeyEventCallbacks) {
//		PostThreadMessage(GetThreadId(m_hotkeyThread.native_handle()),
//			WM_HOTKEY_REGISTER, 0, reinterpret_cast<LPARAM>(hk));
//	}
//
//
//	#else
//	storeStringKeyCodes();
//	// Lock the thread control mutex.  This will be unlocked when the
//	// thread has finished starting, or when it has fully stopped.
//	pthread_mutex_init(&hook_running_mutex, NULL);
//	pthread_mutex_init(&hook_control_mutex, NULL);
//	pthread_cond_init(&hook_control_cond, NULL);
//
//	// Set the logger callback for library output.
//	hook_set_logger_proc(&logger_proc);
//
//	// Set the event callback for uiohook events.
//	hook_set_dispatch_proc(&dispatch_procB);
//
//	// Start the hook and block.
//	// NOTE If EVENT_HOOK_ENABLED was delivered, the status will always succeed.
//	int status = hook_enable();
//	#endif
//}
//
//void hook::stop(const v8::FunctionCallbackInfo<v8::Value>& args) {
//	#ifdef _WIN32
//
//
//	#else
//	hook_stop();
//	pthread_mutex_destroy(&hook_running_mutex);
//	pthread_mutex_destroy(&hook_control_mutex);
//	pthread_cond_destroy(&hook_control_cond);
//	#endif	
//	}
//
