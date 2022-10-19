const libuiohook = require('../build/RelWithDebInfo/node_libuiohook.node')
var _ = require('underscore');

const keys = ["LeftMouseButton", "RightMouseButton", "MiddleMouseButton", "X1MouseButton", "X2MouseButton", "Backspace", "Tab", "Clear", "Enter", "Shift", "ShiftLeft", "ShiftRight", "Control", "ControlLeft", "ControlRight", "Command", "LeftCommand", "RightCommand", "CommandOrControl", "LeftCommandOrControl", "RightCommandOrControl", "Alt", "AltLeft", "AltRight", "Menu", "LeftMenu", "RightMenu", "OSLeft", "OSRight", "Pause", "Capital", "CapsLock", "NumLock", "ScrollLock", "Escape", "Space", "PageUp", "PageDown", "Home", "End", "Left", "Right", "Up", "Down", "Select", "Print", "Execute", "Snapshot", "PrintScreen", "Insert", "Delete", "Help", "Apps", "Sleep", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "Digit0", "Digit1", "Digit2", "Digit3", "Digit4", "Digit5", "Digit6", "Digit7", "Digit8", "Digit9", "KeyA", "KeyB", "KeyC", "KeyD", "KeyE", "KeyF", "KeyG", "KeyH", "KeyI", "KeyJ", "KeyK", "KeyL", "KeyM", "KeyN", "KeyO", "KeyP", "KeyQ", "KeyR", "KeyS", "KeyT", "KeyU", "KeyV", "KeyW", "KeyX", "KeyY", "KeyZ", "Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5", "Numpad6", "Numpad7", "Numpad8", "Numpad9", "NumpadMultiply", "NumpadDivide", "NumpadAdd", "NumpadSubtract", "Separator", "NumpadDecimal", "NumLock", "NumpadEnter", "Semicolon", "Equal", "Comma", "Minus", "Period", "Slash", "Backquote", "BracketLeft", "Backslash", "BracketRight", "Quote", "ArrowUp", "ArrowLeft", "ArrowRight", "ArrowDown", "MediaPlayPause", "MediaTrackPrevious", "MediaTrackNext", "MediaStop"];

function hot_key_callback() {
    console.log('--------------- callback called');
    console.log('');

    const element = document.getElementById("test-message")
    element.innerText = "hot key pressed";
}

libuiohook.startHook();

window.addEventListener('DOMContentLoaded', () => {

    console.log('--------------- step register hotkeys');

    const some_keys = _.sample(keys, 12);

    for (const some_key of some_keys) {
        const binding_key = {
            callback: hot_key_callback,
            key: some_key,
            eventType: Math.random() < 0.5 ? 'registerKeydown' : 'registerKeyup',
            modifiers: {
                alt: Math.random() < 0.5,
                ctrl: Math.random() < 0.5,
                shift: Math.random() < 0.5,
                meta: Math.random() < 0.5
            }
        };
        libuiohook.registerCallback(binding_key);
    }

    const element = document.getElementById("test-message")
    element.innerText = "hot key registered ";

})

window.addEventListener('unload', () => {
    if(Math.random() < 0.5) {
        libuiohook.unregisterAllCallbacks();
    }

    libuiohook.stopHook();
});