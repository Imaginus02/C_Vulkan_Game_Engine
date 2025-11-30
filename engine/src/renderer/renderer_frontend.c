#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/kmemory.h"

// Backend render context
static renderer_backend* backend = 0;

b8 renderer_initialize(const char* application_name, struct platform_state* platform_state) {
	backend = kallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
	
	// TODO: Make this configurable
	renderer_backend_create(RENDERER_BACKEND_VULKAN, platform_state, backend);

	if (!backend->initialize(backend, application_name, platform_state)) {
		KFATAL("Failed to initialize renderer backend, shutting down.");
		return false;
	}
	return true;
}

void renderer_shutdown() {
	if (backend) {
		backend->shutdown(backend);
		renderer_backend_destroy(backend);
		kfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
	}
}

b8 renderer_begin_frame(f32 delta_time) {
	return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
	b8 result = backend->end_frame(backend, delta_time);
	backend->frame_number++;
	return result;
}

void renderer_on_resize(u16 width, u16 height) {
	if (backend) {
		backend->resized(backend, width, height);
	} else {
		KWARN("Renderer backend not initialized, cannot resize to %ix%i", width, height)
	}
}

b8 renderer_draw_frame(render_packet* packet) {

	if (renderer_begin_frame(packet->delta_time)) {
	
		// ENd th frame
		b8 result = renderer_end_frame(packet->delta_time);
		if (!result) {
			KFATAL("renderer_end_frame failed. Application shutting down.");
			return false;
		}
	}

	return true;
}