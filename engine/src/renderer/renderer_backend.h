#pragma once

#include "renderer_types.inl"
#include "resources/resource_types.h"

struct platform_state;

b8 renderer_backend_create(renderer_backend_type type, renderer_backend* out_renderer_backend);
void renderer_backend_destroy(renderer_backend* backend);

void vulkan_renderer_create_texture(const u8* pixels, texture* texture);
void vulkan_renderer_destroy_texture(texture* texture);