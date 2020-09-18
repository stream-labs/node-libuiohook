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

// #include <map>
// #include <mutex>
// #include <list>
// #include <string>
// #include <thread>
// #include <vector>
// #include <inttypes.h>
// #include "uiohook.h"
// #include <algorithm>

// #include <CoreFoundation/CoreFoundation.h>

#include "hook.h"
#include "uiohook.h"

#include <map>
#include <CoreFoundation/CoreFoundation.h>

#define UIOHOOK_ERROR_THREAD_CREATE 0x10

std::map<std::string, int> g_keyCodesArray;
std::map<uint16_t, _event_type> g_modifiers;

struct KeyData {
	int code;
};

struct Event {
	uint16_t key;
	std::map< uint16_t, _event_type> modifiers;
};

struct Action {
	_event_type m_event;
	Event m_codeEvent;
	_event_type m_currentState;
	Worker *m_js_callBack;
};

std::vector<Action*> pressedKeyEventCallbacks;
std::vector<Action*> releasedKeyEventCallbacks;

// Thread and mutex variables.
static pthread_t hook_thread;

static pthread_mutex_t hook_running_mutex;
static pthread_mutex_t hook_control_mutex;
static pthread_cond_t hook_control_cond;

static pthread_mutex_t pressed_keys_mutex;
static pthread_mutex_t released_keys_mutex;

void updateModifierState(uint16_t key, _event_type state) {
	if (key == VC_SHIFT_L || key == VC_SHIFT_R) {
		auto left = g_modifiers.find(VC_SHIFT_L);
		if (left != g_modifiers.end())
			left->second = state;

		auto right = g_modifiers.find(VC_SHIFT_R);
		if (right != g_modifiers.end())
			right->second = state;
	}
	if (key == VC_CONTROL_L || key == VC_CONTROL_R) {
		auto left = g_modifiers.find(VC_CONTROL_L);
		if (left != g_modifiers.end())
			left->second = state;

		auto right = g_modifiers.find(VC_CONTROL_R);
		if (right != g_modifiers.end())
			right->second = state;
	}
	if (key == VC_ALT_L || key == VC_ALT_R) {
		auto left = g_modifiers.find(VC_ALT_L);
		if (left != g_modifiers.end())
			left->second = state;

		auto right = g_modifiers.find(VC_ALT_R);
		if (right != g_modifiers.end())
			right->second = state;
	}
	if (key == VC_META_L || key == VC_META_R) {
		auto left = g_modifiers.find(VC_META_L);
		if (left != g_modifiers.end())
			left->second = state;

		auto right = g_modifiers.find(VC_META_R);
		if (right != g_modifiers.end())
			right->second = state;
	}
}

void storeStringKeyCodes(void) {
	g_keyCodesArray = {
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
		std::make_pair("Numpad1", VC_KP_1),
		std::make_pair("Numpad2", VC_KP_2),
		std::make_pair("Numpad3", VC_KP_3),
		std::make_pair("Numpad4", VC_KP_4),
		std::make_pair("Numpad5", VC_KP_5),
		std::make_pair("Numpad6", VC_KP_6),
		std::make_pair("Numpad7", VC_KP_7),
		std::make_pair("Numpad8", VC_KP_8),
		std::make_pair("Numpad9", VC_KP_9),
		std::make_pair("Numpad0", VC_KP_0),
		std::make_pair("Digit1", VC_1),
		std::make_pair("Digit2", VC_2),
		std::make_pair("Digit3", VC_3),
		std::make_pair("Digit4", VC_4),
		std::make_pair("Digit5", VC_5),
		std::make_pair("Digit6", VC_6),
		std::make_pair("Digit7", VC_7),
		std::make_pair("Digit8", VC_8),
		std::make_pair("Digit9", VC_9),
		std::make_pair("Digit0", VC_0),
		std::make_pair("Backspace", VC_BACKSPACE),
		std::make_pair("Tab", VC_TAB),
		std::make_pair("KeyA", VC_A),
		std::make_pair("KeyB", VC_B),
		std::make_pair("KeyC", VC_C),
		std::make_pair("KeyD", VC_D),
		std::make_pair("KeyE", VC_E),
		std::make_pair("KeyF", VC_F),
		std::make_pair("KeyG", VC_G),
		std::make_pair("KeyH", VC_H),
		std::make_pair("KeyI", VC_I),
		std::make_pair("KeyJ", VC_J),
		std::make_pair("KeyK", VC_K),
		std::make_pair("KeyL", VC_L),
		std::make_pair("KeyM", VC_M),
		std::make_pair("KeyN", VC_N),
		std::make_pair("KeyO", VC_O),
		std::make_pair("KeyP", VC_P),
		std::make_pair("KeyQ", VC_Q),
		std::make_pair("KeyR", VC_R),
		std::make_pair("KeyS", VC_S),
		std::make_pair("KeyT", VC_T),
		std::make_pair("KeyU", VC_U),
		std::make_pair("KeyV", VC_V),
		std::make_pair("KeyW", VC_W),
		std::make_pair("KeyX", VC_X),
		std::make_pair("KeyY", VC_Y),
		std::make_pair("KeyZ", VC_Z),
		std::make_pair("Control", MASK_CTRL),
		std::make_pair("CommandOrControl", MASK_META),
		std::make_pair("Command", MASK_META),
		std::make_pair("Alt", MASK_ALT),
		std::make_pair("Shift", MASK_SHIFT),

		// Keyboard
		std::make_pair("Backspace", VC_BACKSPACE),
		std::make_pair("Tab", VC_TAB),
		std::make_pair("Clear", VC_CLEAR),
		std::make_pair("Enter", VC_ENTER),
		std::make_pair("Menu", VC_CONTEXT_MENU),
		// std::make_pair("LeftMenu", VK_LMENU), std::make_pair("RightMenu", VK_RMENU),
		// std::make_pair("OSLeft", VK_LWIN), std::make_pair("OSRight", VK_RWIN),
		std::make_pair("Pause", VC_PAUSE),
		// std::make_pair("Capital", VK_CAPITAL),
		std::make_pair("CapsLock", VC_CAPS_LOCK),
		std::make_pair("NumLock", VC_NUM_LOCK),
		std::make_pair("ScrollLock", VC_SCROLL_LOCK),
		std::make_pair("Escape", VC_ESCAPE),
		std::make_pair("Space", VC_SPACE),
		std::make_pair("PageUp", VC_KP_PAGE_UP), std::make_pair("PageDown", VC_KP_PAGE_DOWN),
		std::make_pair("Home", VC_KP_HOME), std::make_pair("End", VC_KP_END),
		std::make_pair("Left", VC_KP_LEFT), std::make_pair("Right", VC_KP_RIGHT),
		std::make_pair("Up", VC_KP_UP), std::make_pair("Down", VC_KP_DOWN),
		// std::make_pair("Select", VK_SELECT),
		// std::make_pair("Print", VK_PRINT),
		// std::make_pair("Execute", VK_EXECUTE),
		// std::make_pair("Snapshot", VK_SNAPSHOT),
		std::make_pair("PrintScreen", VC_PRINTSCREEN),
		std::make_pair("Insert", VC_SUN_INSERT), std::make_pair("Delete", VC_DELETE),
		std::make_pair("Help", VC_SUN_HELP),
		// std::make_pair("Apps", VK_APPS),
		std::make_pair("Sleep", VC_SLEEP),

		/// OEM Keys
		std::make_pair("Semicolon", VC_SEMICOLON), std::make_pair("Equal", VC_EQUALS),
		std::make_pair("Comma", VC_COMMA), std::make_pair("Minus", VC_MINUS),
		std::make_pair("Period", VC_PERIOD), std::make_pair("Slash", VC_SLASH),
		std::make_pair("Backquote", VC_BACKQUOTE), std::make_pair("BracketLeft", VC_OPEN_BRACKET),
		std::make_pair("Backslash", VC_BACK_SLASH), std::make_pair("BracketRight", VC_CLOSE_BRACKET),
		std::make_pair("Quote", VC_QUOTE),
		// Arrows
		std::make_pair("ArrowUp", VC_UP), std::make_pair("ArrowLeft", VC_LEFT),
		std::make_pair("ArrowRight", VC_RIGHT), std::make_pair("ArrowDown", VC_DOWN),

		/// Media
		std::make_pair("MediaPlayPause", VC_MEDIA_PLAY), std::make_pair("MediaTrackPrevious", VC_MEDIA_PREVIOUS),
		std::make_pair("MediaTrackNext", VC_MEDIA_NEXT), std::make_pair("MediaStop", VC_MEDIA_STOP)
	};

	g_modifiers = {
		std::make_pair(VC_SHIFT_L, EVENT_KEY_RELEASED),
		std::make_pair(VC_SHIFT_R, EVENT_KEY_RELEASED),
		std::make_pair(VC_CONTROL_L, EVENT_KEY_RELEASED),
		std::make_pair(VC_CONTROL_R, EVENT_KEY_RELEASED),
		std::make_pair(VC_ALT_L, EVENT_KEY_RELEASED),
		std::make_pair(VC_ALT_R, EVENT_KEY_RELEASED),
		std::make_pair(VC_META_L, EVENT_KEY_RELEASED),
		std::make_pair(VC_META_R, EVENT_KEY_RELEASED),
	};
}

void dispatch_procB(uiohook_event * const event) {
	switch (event->type) {
		case EVENT_HOOK_ENABLED:
			// Lock the running mutex so we know if the hook is enabled.
			pthread_mutex_lock(&hook_running_mutex);

			// Unlock the control mutex so hook_enable() can continue.
			pthread_cond_signal(&hook_control_cond);
			pthread_mutex_unlock(&hook_control_mutex);
			break;

		case EVENT_HOOK_DISABLED:
			// Lock the control mutex until we exit.
			pthread_mutex_lock(&hook_control_mutex);

			// Unlock the running mutex so we know if the hook is disabled.
			#ifdef __MACH__
			// Stop the main runloop so that this program ends.
			CFRunLoopStop(CFRunLoopGetMain());
			#endif

			pthread_mutex_unlock(&hook_running_mutex);
			break;

		case EVENT_KEY_PRESSED: {
			pthread_mutex_lock(&pressed_keys_mutex);
			// std::cout << "key code " << event->data.keyboard.keycode << std::endl;
			for (int i = 0; i < pressedKeyEventCallbacks.size(); i++) {
				if (//If the associated event is an EVENT_KEY_PRESSED type
					pressedKeyEventCallbacks.at(i)->m_event == EVENT_KEY_PRESSED &&
					//If the current key pressed is associated with an element in the vector
					event->data.keyboard.keycode == pressedKeyEventCallbacks.at(i)->m_codeEvent.key &&
					//If the key is not already pressed
					pressedKeyEventCallbacks.at(i)->m_currentState != EVENT_KEY_PRESSED) {
					bool hasModifiers = !pressedKeyEventCallbacks.at(i)->m_codeEvent.modifiers.empty();
					bool modifiersPressed = false;

					for (auto modifier: pressedKeyEventCallbacks.at(i)->m_codeEvent.modifiers) {
						auto mod_it = g_modifiers.find(modifier.first);
						if (mod_it != g_modifiers.end() &&
							mod_it->second != EVENT_KEY_PRESSED) {
							modifiersPressed = false;
							break;
						}
						modifiersPressed = true;
					}

					if (hasModifiers == modifiersPressed) {
						pressedKeyEventCallbacks.at(i)->m_js_callBack->Queue();
						pressedKeyEventCallbacks.at(i)->m_currentState = EVENT_KEY_PRESSED;
						break;
					}
				}
			}

			auto mod_it = g_modifiers.find(event->data.keyboard.keycode);
			if (mod_it != g_modifiers.end())
				updateModifierState(event->data.keyboard.keycode, EVENT_KEY_PRESSED);

			pthread_mutex_unlock(&pressed_keys_mutex);
			break;
		}
		case EVENT_KEY_RELEASED: {
			pthread_mutex_lock(&released_keys_mutex);
			for (int i = 0; i < releasedKeyEventCallbacks.size(); i++) {
				if (//If the associated event is an EVENT_KEY_RELEASED type
					releasedKeyEventCallbacks.at(i)->m_event == EVENT_KEY_RELEASED &&
					//If the current key pressed is associated with an element in the vector
					event->data.keyboard.keycode == releasedKeyEventCallbacks.at(i)->m_codeEvent.key) {
						releasedKeyEventCallbacks.at(i)->m_js_callBack->Queue();
						break;
				}
			}

			for (int i = 0; i < pressedKeyEventCallbacks.size(); i++) {
				if (//If the associated event is an EVENT_KEY_PRESSED type
					pressedKeyEventCallbacks.at(i)->m_event == EVENT_KEY_PRESSED &&
					//If the key is already pressed
					pressedKeyEventCallbacks.at(i)->m_currentState == EVENT_KEY_PRESSED) {
					pressedKeyEventCallbacks.at(i)->m_currentState = EVENT_KEY_RELEASED;
					break;
				}
			}

			auto mod_it = g_modifiers.find(event->data.keyboard.keycode);
			if (mod_it != g_modifiers.end())
				updateModifierState(event->data.keyboard.keycode, EVENT_KEY_RELEASED);

			pthread_mutex_unlock(&released_keys_mutex);
			break;
		}
		case EVENT_KEY_TYPED:
		case EVENT_MOUSE_PRESSED:
		case EVENT_MOUSE_RELEASED:
		case EVENT_MOUSE_CLICKED:
		case EVENT_MOUSE_MOVED:
		case EVENT_MOUSE_DRAGGED:
		case EVENT_MOUSE_WHEEL:
		default:
			break;
	}
}

void *hook_thread_proc(void *arg) {
	// Set the hook status.
	int status = hook_run();
	if (status != UIOHOOK_SUCCESS)
		*(int *)arg = status;

	// Make sure we signal that we have passed any exception throwing code for
	// the waiting hook_enable().
	pthread_cond_signal(&hook_control_cond);
	pthread_mutex_unlock(&hook_control_mutex);

	return arg;
}

int hook_enable() {
	// Lock the thread control mutex.  This will be unlocked when the
	// thread has finished starting, or when it has fully stopped.
	pthread_mutex_lock(&hook_control_mutex);

	// Set the initial status.
	int status = UIOHOOK_FAILURE;

	// Create the thread attribute.
	pthread_attr_t hook_thread_attr;
	pthread_attr_init(&hook_thread_attr);

	// Get the policy and priority for the thread attr.
	int policy;
	pthread_attr_getschedpolicy(&hook_thread_attr, &policy);
	int priority = sched_get_priority_max(policy);

	int *hook_thread_status = (int*)malloc(sizeof(int));
	if (pthread_create(&hook_thread, &hook_thread_attr, hook_thread_proc, hook_thread_status) == 0) {
		#if defined(__MACH__) || _POSIX_C_SOURCE >= 200112L
		// Some POSIX revisions do not support pthread_setschedprio so we will 
		// use pthread_setschedparam instead.
		struct sched_param param = { .sched_priority = priority };
		if (pthread_setschedparam(hook_thread, SCHED_OTHER, &param) != 0) {
		}
		#else
		// Raise the thread priority using glibc pthread_setschedprio.
		if (pthread_setschedprio(hook_thread, priority) != 0) {
		}
		#endif


		// Wait for the thread to indicate that it has passed the 
		// initialization portion by blocking until either a EVENT_HOOK_ENABLED 
		// event is received or the thread terminates.
		// NOTE This unlocks the hook_control_mutex while we wait.
		pthread_cond_wait(&hook_control_cond, &hook_control_mutex);

		if (pthread_mutex_trylock(&hook_running_mutex) == 0) {
			// Lock Successful; The hook is not running but the hook_control_cond 
			// was signaled!  This indicates that there was a startup problem!

			// Get the status back from the thread.
			pthread_join(hook_thread, (void **)&hook_thread_status);
			status = *hook_thread_status;
		} else {
			// Lock Failure; The hook is currently running and wait was signaled
			// indicating that we have passed all possible start checks.  We can 
			// always assume a successful startup at this point.
			status = UIOHOOK_SUCCESS;
		}

		free(hook_thread_status);

		} else {
		status = UIOHOOK_ERROR_THREAD_CREATE;
		}

	// Make sure the control mutex is unlocked.
	pthread_mutex_unlock(&hook_control_mutex);

	return status;
}

bool logger_proc(unsigned int level, const char *format, ...) {
	bool status = false;

	return status;
}

Napi::Value StartHotkeyThreadJS(const Napi::CallbackInfo& info) {
	storeStringKeyCodes();
	// Lock the thread control mutex.  This will be unlocked when the
	// thread has finished starting, or when it has fully stopped.
	pthread_mutex_init(&hook_running_mutex, NULL);
	pthread_mutex_init(&hook_control_mutex, NULL);
	pthread_cond_init(&hook_control_cond, NULL);

	// Set the logger callback for library output.
	hook_set_logger_proc(&logger_proc);

	// Set the event callback for uiohook events.
	hook_set_dispatch_proc(&dispatch_procB);

	// Start the hook and block.
	// NOTE If EVENT_HOOK_ENABLED was delivered, the status will always succeed.
	int status = hook_enable();
}

Napi::Value StopHotkeyThreadJS(const Napi::CallbackInfo& info) {
	hook_stop();
	pthread_mutex_destroy(&hook_running_mutex);
	pthread_mutex_destroy(&hook_control_mutex);
	pthread_cond_destroy(&hook_control_cond);
}

Napi::Value RegisterHotkeyJS(const Napi::CallbackInfo& info) {
	Action *action = new Action();

	Napi::Object binds = info[0].ToObject();
	Event event;

	std::string key_str = binds.Get("key").ToString().Utf8Value();
	auto key_it = g_keyCodesArray.find(key_str);
	if (key_it == g_keyCodesArray.end()) {
		std::cout << "Key not found!, key received: " << key_str.c_str() << std::endl;
		return Napi::Boolean::New(info.Env(), false);
	}

	event.key = key_it->second;

	bool modShift, modCtrl, modAlt, modMeta;
	Napi::Object modifiers = binds.Get("modifiers").ToObject();
	modShift = modifiers.Get("shift").ToBoolean().Value();
	modCtrl = modifiers.Get("ctrl").ToBoolean().Value();
	modAlt = modifiers.Get("alt").ToBoolean().Value();
	modMeta = modifiers.Get("meta").ToBoolean().Value();

	if(modShift) {
		event.modifiers.emplace(std::make_pair(VC_SHIFT_L, EVENT_KEY_RELEASED));
		event.modifiers.emplace(std::make_pair(VC_SHIFT_R, EVENT_KEY_RELEASED));
	}

	if(modCtrl) {
		event.modifiers.emplace(std::make_pair(VC_CONTROL_L, EVENT_KEY_RELEASED));
		event.modifiers.emplace(std::make_pair(VC_CONTROL_R, EVENT_KEY_RELEASED));
	}

	if(modAlt) {
		event.modifiers.emplace(std::make_pair(VC_ALT_L, EVENT_KEY_RELEASED));
		event.modifiers.emplace(std::make_pair(VC_ALT_R, EVENT_KEY_RELEASED));
	}

	if(modMeta) {
		event.modifiers.emplace(std::make_pair(VC_META_L, EVENT_KEY_RELEASED));
		event.modifiers.emplace(std::make_pair(VC_META_R, EVENT_KEY_RELEASED));
	}

	action->m_codeEvent = event;

	std::string eventString = binds.Get("eventType").ToString().Utf8Value();

	action->m_currentState = EVENT_KEY_RELEASED;
	auto fnc = binds.Get("callback").As<Napi::Function>();
	action->m_js_callBack = new Worker(fnc);
	action->m_js_callBack->SuppressDestruct();

	if (eventString.compare("registerKeydown") == 0) {
		action->m_event = EVENT_KEY_PRESSED;
		pthread_mutex_lock(&pressed_keys_mutex);
		pressedKeyEventCallbacks.push_back(action);
		pthread_mutex_unlock(&pressed_keys_mutex);
	} else if (eventString.compare("registerKeyup") == 0) {
		action->m_event = EVENT_KEY_RELEASED;
		pthread_mutex_lock(&released_keys_mutex);
		releasedKeyEventCallbacks.push_back(action);
		pthread_mutex_unlock(&released_keys_mutex);
	} else {
		std::cout << "Invalid event receive: " << eventString.c_str() << std::endl;
		return Napi::Boolean::New(info.Env(), false);
	}

	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value UnregisterHotkeyJS(const Napi::CallbackInfo& info) {
	//TODO
}

Napi::Value UnregisterHotkeysJS(const Napi::CallbackInfo& info) {
	pthread_mutex_lock(&pressed_keys_mutex);
	pthread_mutex_lock(&released_keys_mutex);

	pressedKeyEventCallbacks.clear();
	releasedKeyEventCallbacks.clear();

	pthread_mutex_unlock(&pressed_keys_mutex);
	pthread_mutex_unlock(&released_keys_mutex);
}