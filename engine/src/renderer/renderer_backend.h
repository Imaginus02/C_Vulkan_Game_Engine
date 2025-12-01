#pragma once

#include "renderer_types.inl"
#include "resources/resource_types.h"

struct platform_state;

b8 renderer_backend_create(renderer_backend_type type, renderer_backend* out_renderer_backend);
void renderer_backend_destroy(renderer_backend* backend);

void vulkan_renderer_create_texture(const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, texture* out_texture);
void vulkan_renderer_destroy_texture(texture* texture);