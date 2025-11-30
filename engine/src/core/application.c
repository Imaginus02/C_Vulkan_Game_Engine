#include "platform/platform.h"

#include "application.h"
#include "game_types.h"

#include "core/logger.h"
#include "core/kmemory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "memory/linear_allocator.h"

#include "renderer/renderer_frontend.h"

typedef struct application_state {
	game* game_inst;
	b8 is_running;
	b8 is_suspended;
	i16 width;
	i16 height;
	clock clock;
	f64 last_time;

	linear_allocator systems_allocator;

	u64 event_system_memory_requirement;
    void* event_system_state;

    u64 memory_system_memory_requirement;
    void* memory_system_state;

    u64 logging_system_memory_requirement;
    void* logging_system_state;

	u64 input_system_memory_requirement;
    void* input_system_state;

	u64 platform_system_memory_requirement;
    void* platform_system_state;

	u64 renderer_system_memory_requirement;
    void* renderer_system_state;

} application_state;

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_resize(u16 code, void* sender, void* listener_inst, event_context context);

static application_state* app_state;

b8 application_create(game* game_inst)
{
	if (game_inst->application_state) {
		KERROR("application_create called more than once");
		return false;
	}

	game_inst->application_state = kallocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
    app_state = game_inst->application_state;
    app_state->game_inst = game_inst;
    app_state->is_running = false;
    app_state->is_suspended = false;

    u64 systems_allocator_total_size = 64 * 1024 * 1024;  // 64 mb
    linear_allocator_create(systems_allocator_total_size, 0, &app_state->systems_allocator);

    // Events
    event_system_initialize(&app_state->event_system_memory_requirement, 0);
    app_state->event_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->event_system_memory_requirement);
    event_system_initialize(&app_state->event_system_memory_requirement, app_state->event_system_state);

    // Memory
    memory_system_initialize(&app_state->memory_system_memory_requirement, 0);
    app_state->memory_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->memory_system_memory_requirement);
    memory_system_initialize(&app_state->memory_system_memory_requirement, app_state->memory_system_state);

    // Logging
    initialize_logging(&app_state->logging_system_memory_requirement, 0);
    app_state->logging_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->logging_system_memory_requirement);
    if (!initialize_logging(&app_state->logging_system_memory_requirement, app_state->logging_system_state)) {
        KERROR("Failed to initialize logging system; shutting down.");
        return false;
    }
	
    // Input
    input_system_initialize(&app_state->input_system_memory_requirement, 0);
    app_state->input_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->input_system_memory_requirement);
    input_system_initialize(&app_state->input_system_memory_requirement, app_state->input_system_state);

    // Register for engine-level events.
	event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
	event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
	event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
	event_register(EVENT_CODE_RESIZED, 0, application_on_resize);

    // Platform
    platform_system_startup(&app_state->platform_system_memory_requirement, 0, 0, 0, 0, 0, 0);
    app_state->platform_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->platform_system_memory_requirement);
    if (!platform_system_startup(
            &app_state->platform_system_memory_requirement,
            app_state->platform_system_state,
		game_inst->app_config.name,
		game_inst->app_config.start_pos_x,
		game_inst->app_config.start_pos_y,
		game_inst->app_config.width,
		game_inst->app_config.height))
	{
		return false;
	} else {
		KINFO("Platform started successfully");

	}

    // Renderer system
    renderer_system_initialize(&app_state->renderer_system_memory_requirement, 0, 0);
    app_state->renderer_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->renderer_system_memory_requirement);
    if (!renderer_system_initialize(&app_state->renderer_system_memory_requirement, app_state->renderer_system_state, game_inst->app_config.name)) {
		KFATAL("Failed to initialize renderer, shutting down.");
		return false;
	}

	// Initialize the game
	if (!app_state->game_inst->initialize(app_state->game_inst)) {
		KERROR("Failed to initialize game");
		return false;
	}

	// Call resize once to ensure the proper size has been set.
	app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);

	return true;
}

b8 application_run()
{
    app_state->is_running = true;
    clock_start(&app_state->clock);
    clock_update(&app_state->clock);
    app_state->last_time = app_state->clock.elapsed;

	f64 running_time = 0.0;
	u8 frame_count = 0;
	f64 target_frame_seconds = 1.0f / 60;

	//We are technically leaking memory here, but it's just called once
	KINFO(get_memory_usage_str());

	while (app_state->is_running)
	{
		if (!platform_pump_messages()) {
			app_state->is_running = false;
		}

		if (!app_state->is_suspended) {
			// Update the clock and get the delta time
			clock_update(&app_state->clock);
			f64 current_time = app_state->clock.elapsed;
			f64 delta = (current_time - app_state->last_time);
			f64 frame_start_time = platform_get_absolute_time();

			if (!app_state->game_inst->update(app_state->game_inst, (f32)delta)) {
				KFATAL("Failed to update game, shutting down.");
				app_state->is_running = false;
				break;
			}

			// Call the game's renderer routine
			if (!app_state->game_inst->render(app_state->game_inst, (f32)delta)) {
				KFATAL("Failed to render game, shutting down.");
				app_state->is_running = false;
				break;
			}

			// TODO: Refactor the packet creation
			render_packet packet;
			packet.delta_time = (f32)delta;
			renderer_draw_frame(&packet);

			// Figure out how long the frame took
			f64 frame_end_time = platform_get_absolute_time();
			f64 frame_elapsed = frame_end_time - frame_start_time;
			running_time += frame_elapsed;
			// How long we should wait before the next frame
			f64 remaining_time = target_frame_seconds - frame_elapsed;

			if (remaining_time > 0.0) {
				// If there is time left, sleep for the remaining time
				b8 limit_frames = false;
				if (remaining_time > 0.0 && limit_frames) {
					platform_sleep((u64)(remaining_time * 1000.0)-1);
				}

				frame_count++;
			}

			// NOTE: Input update/sate copying sgould always be handled after any input should be recorded
			// As a safety, input is the last thing to be updated before the frame ends
			input_update(delta);

			app_state->last_time = current_time;
		}
	}

	app_state->is_running = false;

	event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
	event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
	event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
	event_unregister(EVENT_CODE_RESIZED, 0, application_on_resize);

	input_system_shutdown(app_state->input_system_state);

    renderer_system_shutdown(app_state->renderer_system_state);

    platform_system_shutdown(app_state->platform_system_state);

	memory_system_shutdown(app_state->memory_system_state);

	event_system_shutdown(app_state->event_system_state);
	return true;
}

void application_get_framebuffer_size(u32* width, u32* height)
{
	*width = app_state->width;
	*height = app_state->height;
}


b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context)
{
	switch (code)
	{
		case EVENT_CODE_APPLICATION_QUIT:
		{
			KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down");
			app_state->is_running = false;
			return true;
		}
	}

	return false;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context)
{
	if (code == EVENT_CODE_KEY_PRESSED) 
	{
		u16 key_code = context.data.u16[0];
		if (key_code == KEY_ESCAPE) 
		{
			// NOTE: Technically firing an event to himself, but there are maybe other listeners
			event_context data = {};
			event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
			return true;
		} else if (key_code == KEY_A){
			KDEBUG("Key A pressed");
		} else {
			KDEBUG("Key %c pressed", key_code);
		}
	} else if (code == EVENT_CODE_KEY_RELEASED) {
		u16 key_code = context.data.u16[0];
		if (key_code == KEY_B) {
			KDEBUG("Key B released");
		}
#if defined(_DEBUG)
		 else if (key_code == KEY_M){
			KDEBUG(get_memory_usage_str());
		}
#endif 
		else {
			KDEBUG("Key %c released", key_code);
		}
	}
	return false;
}

b8 application_on_resize (u16 code, void* sender, void* listener_inst, event_context context)
{
	if (code == EVENT_CODE_RESIZED) {
		u16 width = context.data.u16[0];
		u16 height = context.data.u16[1];

		if (width != app_state->width || height != app_state->height) {
			app_state->width = width;
			app_state->height = height;
			KDEBUG("Window resized to %ix%i", width, height);

			// Handle minimization
			if (width == 0 || height == 0) {
				KINFO("Window minimized, suspending the Application.");
				app_state->is_suspended = true;
				return true;
			} else {
				if (app_state->is_suspended) {
					KINFO("Window restored, resuming the Application.");
					app_state->is_suspended = false;
				}
				app_state->game_inst->on_resize(app_state->game_inst, width, height);
				renderer_on_resize(width, height);
			}
		}
	}
	// Event purposely not handled
	return false;
}