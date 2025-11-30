#include "kmemory.h"

#include "core/logger.h"
#include "platform/platform.h"

// TODO: custom string library
#include <string.h>
#include <stdio.h>

struct memory_stats {
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
	"UNKNOWN          ",
	"ARRAY            ",
	"LINEAR_ALLOCATOR ",
	"DARRAY           ",
	"DICT             ",
	"RING_QUEUE       ",
	"BST              ",
	"STRING           ",
	"APPLICATION      ",
	"JOB              ",
	"TEXTURE          ",
	"MATERIAL_INSTANCE",
	"RENDERER         ",
	"GAME             ",
	"TRANSFORM        ",
	"ENTITY           ",
	"ENTITY_NODE      ",
	"SCENE            "
};

typedef struct memory_system_state {
    struct memory_stats stats;
    u64 alloc_count;
} memory_system_state;

// TODO: implement a memory struct to keep track of the size of each allocation and not have to put the tag and size in free

static memory_system_state* state_ptr;

void initialize_memory(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(memory_system_state);
    if (state == 0) {
        return;
    }

    state_ptr = state;
    state_ptr->alloc_count = 0;
    platform_zero_memory(&state_ptr->stats, sizeof(state_ptr->stats));
}

void shutdown_memory(void* state) {
    state_ptr = 0;
}

void* kallocate(u64 size, memory_tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation")
	}
	if (state_ptr) {
        state_ptr->stats.total_allocated += size;
        state_ptr->stats.tagged_allocations[tag] += size;
        state_ptr->alloc_count++;
    }

	// TODO: Add memory alignment
	void* block = platform_allocate(size, false);
	platform_zero_memory(block, size);
	return block;
}

void kfree(void* block, u64 size, memory_tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation")
	}

	if (state_ptr) {
		state_ptr->stats.total_allocated -= size;
		state_ptr->stats.tagged_allocations[tag] -= size;
	}

	platform_free(block, false);
}

void* kzero_memory(void* block, u64 size) {
	return platform_zero_memory(block, size);
}

void* kcopy_memory(void* dest, const void* src, u64 size) {
	return platform_copy_memory(dest, src, size);
}

void* kset_memory(void* dest, i32 value, u64 size) {
	return platform_set_memory(dest, value, size);
}

char* get_memory_usage_str() {
	// We want to output the memory usage in KB/MB/GB/...
	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;
	
	// TODO: determine the size of the buffer
	char buffer[8000] = "System memory use (tagged):\n";
	u64 offset = strlen(buffer);

	for (u32 i =0; i< MEMORY_TAG_MAX_TAGS; i++) {

		// Decide the unit
		char unit[4] = "XiB";
		float amount = 1.0f;
		if (state_ptr->stats.tagged_allocations[i] > gib) {
			unit[0] = 'G';
			amount = state_ptr->stats.tagged_allocations[i] / (float)gib;
		} else if (state_ptr->stats.tagged_allocations[i] > mib) {
			unit[0] = 'M';
			amount = state_ptr->stats.tagged_allocations[i] / (float)mib;
		} else if (state_ptr->stats.tagged_allocations[i] > kib) {
			unit[0] = 'K';
			amount = state_ptr->stats.tagged_allocations[i] / (float)kib;
		} else {
			unit[0] = 'B';
			unit[1] = '\0';
			amount = state_ptr->stats.tagged_allocations[i];
		}

		i32 length = snprintf(buffer + offset, 8000 - offset, "%s: %.2f %s\n", memory_tag_strings[i], amount, unit);
		offset += length;
	}
	//On renvoit une copie du buffer pour éviter qu'il soit détruit à la fin de la fonction
	//Il faudra penser à le libérer après utilisation (après l'avoir affiché par exemple)
	char* out_string = _strdup(buffer);
    return out_string;
}

u64 get_memory_alloc_count() {
    if (state_ptr) {
        return state_ptr->alloc_count;
    }
    return 0;
}