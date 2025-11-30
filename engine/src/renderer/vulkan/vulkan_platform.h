#pragma once

#include "defines.h"

struct platform_state;
struct vulkan_context;

b8 platform_create_vulkan_surface(struct vulkan_context* context);


/**
 * Appends the name of required extensions for this platform to
 * the names_darray.
 * 
 * @param names_darray The dynamic array to append the names to.
 */
void platform_get_required_extension_names(const char*** names_darray);