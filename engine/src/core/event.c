#include "core/event.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "containers/darray.h"

typedef struct registered_event {
	void* listener;
	PFN_on_event callback;
} registered_event;

typedef struct event_code_entr {
	registered_event* events;
} event_code_entry;

// The maximum number of event codes that can be registered.
#define MAX_MESSAGE_CODES 16384

// State structure
typedef struct event_system_state {
	event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

/**
 * Event system internal state_ptr->
 */
static b8 is_initialized = false;
static event_system_state* state_ptr;

void event_system_initialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(event_system_state);
    if (state == 0) {
        return;
	}

    kzero_memory(state, sizeof(state));
    state_ptr = state;
}

void event_system_shutdown(void* state) {
    if (state_ptr) {
        // Free the events arrays. And objects pointed to should be destroyed on their own.
        for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
            if (state_ptr->registered[i].events != 0) {
                darray_destroy(state_ptr->registered[i].events);
                state_ptr->registered[i].events = 0;
            }
        }
    }
    state_ptr = 0;
}

b8 event_register(u16 code, void* listener, PFN_on_event on_event)
{
	if (!state_ptr) {
		KERROR("Event system not initialized.");
		return false;
	}

    if (state_ptr->registered[code].events == 0) {
        state_ptr->registered[code].events = darray_create(registered_event);
	}

    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        if (state_ptr->registered[code].events[i].listener == listener) {
			KWARN("Event listener already registered.");
			return false;
		}
	}

	// If we got here, the listener is not registered yet, so we can add it.
	registered_event new_event;
	new_event.listener = listener;
	new_event.callback = on_event;

	darray_push(state_ptr->registered[code].events, new_event);

	return true;
}

b8 event_unregister(u16 code, void* listener, PFN_on_event on_event)
{
	if (!state_ptr) {
		KERROR("Event system not initialized.");
		return false;
	}

	if (state_ptr->registered[code].events == 0) {
		KWARN("No events registered for this code.");
		return false;
	}

	u64 registered_count = darray_length(state_ptr->registered[code].events);
	for (u64 i = 0; i < registered_count; ++i) {
		registered_event e = state_ptr->registered[code].events[i];
		if (e.listener == listener) {
			registered_event popped_event;
			darray_pop_at(state_ptr->registered[code].events, i, &popped_event);
			return true;
		}
	}

	KWARN("Event listener not registered.");
	return false;
}

b8 event_fire(u16 code, void* sender, event_context context)
{
	if (!state_ptr) {
		KERROR("Event system not initialized.");
		return false;
	}

	if (state_ptr->registered[code].events == 0) {
		// There are no listeners for this event
		// This is doesn't have to be an error or warning
		return false;
	}

	u64 registered_count = darray_length(state_ptr->registered[code].events);
	// KDEBUG("Firing event %d to %d listeners", code, registered_count);
	for (u64 i = 0; i < registered_count; ++i) {
		registered_event e = state_ptr->registered[code].events[i];
		if (e.callback(code, sender, e.listener, context)) {
			return true;
		}
	}

	return false;
}