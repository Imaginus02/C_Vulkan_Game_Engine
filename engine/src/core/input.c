#include "core/input.h"
#include "core/event.h"
#include "core/kmemory.h"
#include "core/logger.h"

typedef struct keyboard_state {
	b8 keys[256];
} keyboard_state;

typedef struct mouse_state {
	i16 x;
	i16 y;
	b8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
	keyboard_state keyboard_previous;
	keyboard_state keyboard_current;
	mouse_state mouse_previous;
	mouse_state mouse_current;
} input_state;

static b8 initialized = false;
static input_state state = {};

void input_initialize() {
	if (initialized) {
		KWARN("Input already initialized");
		return;
	}

	kzero_memory(&state, sizeof(input_state));
	initialized = true;
	KINFO("Input subsystem initialized");
}

void input_shutdown() {
	if (!initialized) {
		KWARN("Trying to shutdown input subsystem that has not been initialized");
		return;
	}

	initialized = false;
	KINFO("Input subsystem shut down");
}

void input_update(f64 delta_time) {
	if (!initialized) {
		KERROR("Trying to update input subsystem that has not been initialized");
		return;
	}

	// Copy the current state to the previous state
	kcopy_memory(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
	kcopy_memory(&state.mouse_previous, &state.mouse_current, sizeof(mouse_state));
}

void input_process_key(keys key, b8 pressed) {
	if (key == KEY_LALT) {
        KINFO("Left alt pressed.");
    } else if (key == KEY_RALT) {
        KINFO("Right alt pressed.");
    }

    if (key == KEY_LCONTROL) {
        KINFO("Left ctrl pressed.");
    } else if (key == KEY_RCONTROL) {
        KINFO("Right ctrl pressed.");
    }

    if (key == KEY_LSHIFT) {
        KINFO("Left shift pressed.");
    } else if (key == KEY_RSHIFT) {
        KINFO("Right shift pressed.");
    }

	// Only handle this if the state actually changed
	if (state.keyboard_current.keys[key] != pressed) {
		// Update the current state
		state.keyboard_current.keys[key] = pressed;

		//Fire off an event for immediate handling
		event_context context;
		context.data.u16[0] = key;

		// TODO: Create a new sender for user input
		event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
	}
}

void input_process_button(buttons button, b8 pressed) {
	// Only handle this if the state actually changed
	if (state.mouse_current.buttons[button] != pressed) {
		// Update the current state
		state.mouse_current.buttons[button] = pressed;

		//Fire off an event for immediate handling
		event_context context;
		context.data.u16[0] = button;
		event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
	}
}

void input_process_mouse_move(i16 x, i16 y) {
	// Only handle this if the state actually changed
	if (state.mouse_current.x != x || state.mouse_current.y != y) {
		// NOTE: Enable this if debugging
		// KDEBUG("Mouse moved to (%i, %i)", x, y);
		
		// Update the current state
		state.mouse_current.x = x;
		state.mouse_current.y = y;

		//Fire off an event for immediate handling
		event_context context;
		context.data.i16[0] = x;
		context.data.i16[1] = y;
		event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
	}
}

void input_process_mouse_wheel(i8 delta) {
	//There is no need to check if the state changed for the mouse wheel

	//Fire off an event for immediate handling
	event_context context;
	context.data.i16[0] = delta;
	event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_is_key_down(keys key) {
	if (!initialized) {
		KERROR("Trying to check if a key is down on an uninitialized input subsystem");
		return false;
	}

	return state.keyboard_current.keys[key] == true;
}

b8 input_is_key_up(keys key) {
	if (!initialized) {
		KERROR("Trying to check if a key is up on an uninitialized input subsystem");
		return false;
	}

	return state.keyboard_current.keys[key] == false;
}

b8 input_was_key_down(keys key) {
	if (!initialized) {
		KERROR("Trying to check if a key was down on an uninitialized input subsystem");
		return false;
	}

	return state.keyboard_previous.keys[key] == true;
}

b8 input_was_key_up(keys key) {
	if (!initialized) {
		KERROR("Trying to check if a key was up on an uninitialized input subsystem");
		return false;
	}

	return state.keyboard_previous.keys[key] == false;
}

b8 input_is_button_down(buttons button) {
	if (!initialized) {
		KERROR("Trying to check if a button is down on an uninitialized input subsystem");
		return false;
	}

	return state.mouse_current.buttons[button] == true;
}

b8 input_is_button_up(buttons button) {
	if (!initialized) {
		KERROR("Trying to check if a button is up on an uninitialized input subsystem");
		return false;
	}

	return state.mouse_current.buttons[button] == false;
}

b8 input_was_button_down(buttons button) {
	if (!initialized) {
		KERROR("Trying to check if a button was down on an uninitialized input subsystem");
		return false;
	}

	return state.mouse_previous.buttons[button] == true;
}

b8 input_was_button_up(buttons button) {
	if (!initialized) {
		KERROR("Trying to check if a button was up on an uninitialized input subsystem");
		return false;
	}

	return state.mouse_previous.buttons[button] == false;
}

void input_get_mouse_position(i32* x, i32* y) {
	if (!initialized) {
		KERROR("Trying to get the mouse position on an uninitialized input subsystem");
		return;
	}

	*x = state.mouse_current.x;
	*y = state.mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
	if (!initialized) {
		KERROR("Trying to get the previous mouse position on an uninitialized input subsystem");
		return;
	}

	*x = state.mouse_previous.x;
	*y = state.mouse_previous.y;
}