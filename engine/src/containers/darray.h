#pragma once

#include "defines.h"

/**
 * @brief Python's like array
 * 
 * Memory layout:
 * - u64 capacity: number of elements that can be held
 * - u64 length: number of elements that are currently contained
 * - u64 stride: size of each element in bytes
 * - void* elements: pointer to the elements
 */
enum {
	DARRAY_CAPACITY,
	DARRAY_LENGTH,
	DARRAY_STRIDE,
	DARRAY_FIELD_LENGTH
};

/**
 * @brief Create a dynamic array with the specified length and stride.
 * 
 * @param length The initial length of the array.
 * @param stride The size of each element in bytes.
 * @return void* A pointer to the created dynamic array.
 */
KAPI void* _darray_create(u64 length, u64 stride);

/**
 * @brief Destroy a dynamic array.
 * 
 * @param darray A pointer to the dynamic array to destroy.
 */
KAPI void _darray_destroy(void* darray);

/**
 * @brief Get the value of a field in the dynamic array.
 * 
 * @param darray A pointer to the dynamic array.
 * @param field The field to get the value of.
 * @return u64 The value of the specified field.
 */
KAPI u64 _darray_field_get(void* darray, u64 field);

/**
 * @brief Set the value of a field in the dynamic array.
 * 
 * @param darray A pointer to the dynamic array.
 * @param field The field to set the value of.
 * @param value The value to set.
 */
KAPI void _darray_field_set(void* darray, u64 field, u64 value);

/**
 * @brief Resize a dynamic array to increase its capacity.
 * 
 * @param darray A pointer to the dynamic array to resize.
 * @return void* A pointer to the resized dynamic array.
 */
KAPI void* _darray_resize(void* darray);

/**
 * @brief Push a value to the end of a dynamic array.
 * 
 * @param darray A pointer to the dynamic array.
 * @param value_ptr A pointer to the value to push.
 * @return void* A pointer to the modified dynamic array.
 */
KAPI void* _darray_push(void* darray, const void* value_ptr);

/**
 * @brief Pop the last value from a dynamic array.
 * 
 * @param darray A pointer to the dynamic array.
 * @param dest A pointer to store the popped value.
 */
KAPI void _darray_pop(void* darray, void* dest);

/**
 * @brief Insert a value at the specified index in a dynamic array.
 * 
 * @param darray A pointer to the dynamic array.
 * @param index The index at which to insert the value.
 * @param value_ptr A pointer to the value to insert.
 * @return void* A pointer to the modified dynamic array.
 */
KAPI void* _darray_insert_at(void* darray, u64 index, void* value_ptr);

/**
 * @brief Pop the value at the specified index from a dynamic array.
 * 
 * @param darray A pointer to the dynamic array.
 * @param index The index of the value to pop.
 * @param dest A pointer to store the popped value.
 * @return void* A pointer to the modified dynamic array.
 */
KAPI void* _darray_pop_at(void* darray, u64 index, void* dest);

/**
 * @brief Default capacity of the dynamic array when it is created.
 */
#define DARRAY_DEFAULT_CAPACITY 1

/**
 * @brief The factor by which the dynamic array is resized when it is full.
 */
#define DARRAY_RESIZE_FACTOR 2

/**
 * @brief Create a dynamic array of the specified type with the default capacity.
 * 
 * @param type The type of the elements in the array.
 * @return void* A pointer to the created dynamic array.
 */
#define darray_create(type) \
	_darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

/**
 * @brief Create a dynamic array of the specified type with the specified capacity.
 * 
 * @param type The type of the elements in the array.
 * @param capacity The initial capacity of the array.
 * @return void* A pointer to the created dynamic array.
 */
#define darray_reserve(type, capacity) \
	_darray_create(capacity, sizeof(type))

/**
 * @brief Destroy a dynamic array.
 * 
 * @param darray A pointer to the dynamic array to destroy.
 */
#define darray_destroy(darray) _darray_destroy(darray);

/**
 * @brief Push a value to the end of a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @param value The value to push.
 */
#define darray_push(array, value) \
	{ \
		typeof(value) temp = value; \
		array = _darray_push(array, &temp); \
	}

/**
 * @brief Pop the last value from a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @param dest A pointer to store the popped value.
 */
#define darray_pop(array, dest) _darray_pop(array, dest);

/**
 * @brief Insert a value at the specified index in a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @param index The index at which to insert the value.
 * @param value The value to insert.
 */
#define darray_insert_at(array, index, value) \
	{ \
		typeof(value) temp = value; \
		array = _darray_insert_at(array, index, &temp); \
	}

/**
 * @brief Pop the value at the specified index from a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @param index The index of the value to pop.
 * @param dest A pointer to store the popped value.
 */
#define darray_pop_at(array, index, dest) _darray_pop_at(array, index, dest);

/**
 * @brief Clear a dynamic array by setting its length to 0.
 * 
 * @param array A pointer to the dynamic array.
 */
#define darray_clear(array) \
	_darray_field_set(array, DARRAY_LENGTH, 0)

/**
 * @brief Get the capacity of a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @return u64 The capacity of the array.
 */
#define darray_capacity(array) \
	_darray_field_get(array, DARRAY_CAPACITY)

/**
 * @brief Get the length of a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @return u64 The length of the array.
 */
#define darray_length(array) \
	_darray_field_get(array, DARRAY_LENGTH)

/**
 * @brief Get the stride of a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @return u64 The stride of the array.
 */
#define darray_stride(array) \
	_darray_field_get(array, DARRAY_STRIDE)

/**
 * @brief Set the length of a dynamic array.
 * 
 * @param array A pointer to the dynamic array.
 * @param length The new length of the array.
 */
#define darray_length_set(array, length) \
	_darray_field_set(array, DARRAY_LENGTH, length)