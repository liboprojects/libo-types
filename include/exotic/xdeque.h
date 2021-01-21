
/**
    \copyright MIT License Copyright (xcapacity) 2020, Adewale Azeez 
    \author Adewale Azeez <azeezadewale98@gmail.com>
    \date 21 December 2020
    \file xdeque.h
*/

#ifndef EXOTIC_XDEQUE_H
#define EXOTIC_XDEQUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xcommon.h"
#include "xiterator.h"

#ifdef __cplusplus
#if !defined(ALLOW_X_TYPES_WITH_ALTERNATIVES_IN_CPP) && __cplusplus >= 201103L
    #warning Do not use this type in C++ 03 and above, use the std::container class instead
#endif
#define NULL 0
#endif

#define SETUP_XDEQUE_FOR(T) typedef struct xdeque_##T##_s { \
    size_t capacity;\
    size_t expansion_rate;\
    size_t size;\
    size_t max_size;\
    size_t first;\
    size_t last;\
    T *buffer;\
    XIterator *iter;\
    void *(*memory_alloc)  (size_t size);\
    void *(*memory_calloc) (size_t blocks, size_t size);\
    void  (*memory_free)   (void *block);\
} xdeque_##T;\
\
enum x_stat xdeque_##T##_new(xdeque_##T **out);\
enum x_stat xdeque_##T##_new_config(struct xcontainer_config * const config, xdeque_##T **out);\
enum x_stat xdeque_##T##_add(xdeque_##T *container, T element);\
static enum x_stat xdeque_##T##_expand_capacity(xdeque_##T *container);\
\
enum x_stat xdeque_##T##_new(xdeque_##T **out) \
{\
    struct xcontainer_config config;\
    init_xcontainer_config(&config);\
    return xdeque_##T##_new_config(&config, out);\
}\
\
enum x_stat xdeque_##T##_new_max_size(xdeque_##T **out, size_t max_size) \
{\
    struct xcontainer_config config;\
    init_xcontainer_config_max_size(&config, max_size);\
    return xdeque_##T##_new_config(&config, out);\
}\
\
enum x_stat xdeque_##T##_new_config(struct xcontainer_config * const config, xdeque_##T **out) \
{\
    size_t expansion_rate;\
    xdeque_##T *container;\
    T *buffer;\
    XIterator *iter;\
    if (config->expansion_rate <= 1) {\
        expansion_rate = XDEFAULT_CONTAINER_EXPANSION_RATE;\
    } else {\
        expansion_rate = config->expansion_rate;\
    }\
    if ((!config->capacity || expansion_rate >= (config->max_size / config->capacity)) && (config->max_size < config->capacity)) {\
        return X_INVALID_CAPACITY_ERR;\
    }\
    container = (xdeque_##T *) config->memory_calloc(1, sizeof(xdeque_##T));\
    if (!container) {\
        return X_ALLOC_ERR;\
    }\
    buffer = (T *) config->memory_alloc(config->capacity * sizeof(T));\
    if (!buffer) {\
        config->memory_free(container);\
        return X_ALLOC_ERR;\
    }\
    iter = (XIterator *) config->memory_alloc(sizeof(XIterator));\
    if (!iter) {\
        config->memory_free(buffer);\
        config->memory_free(container);\
        return X_ALLOC_ERR;\
    }\
    container->capacity             = x_upper_pow_two(config->capacity);\
    container->expansion_rate       = config->expansion_rate;\
    container->max_size             = config->max_size;\
    container->size                 = 0;\
    container->first                = 0;\
    container->last                 = 0;\
    container->memory_alloc         = config->memory_alloc;\
    container->memory_calloc        = config->memory_calloc;\
    container->memory_free          = config->memory_free;\
    container->buffer               = buffer;\
    container->iter                 = iter;\
    container->iter->index          = 0;\
    container->iter->backward_index = 0;\
    *out = container;\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_add_front(xdeque_##T *container, T element)\
{\
    if (container->size >= container->max_size) {\
        return X_MAX_SIZE_ERR;\
    }\
    if (container->size >= container->capacity && xdeque_##T##_expand_capacity(container) != X_OK) {\
        return X_ALLOC_ERR;\
    }\
    container->first = (container->first - 1) & (container->capacity - 1);\
    container->buffer[container->first] = element;\
    container->size++;\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_add_back(xdeque_##T *container, T element)\
{\
    if (container->size >= container->max_size) {\
        return X_MAX_SIZE_ERR;\
    }\
    if (container->capacity == container->size && xdeque_##T##_expand_capacity(container) != X_OK) {\
        return X_ALLOC_ERR;\
    }\
    container->buffer[container->last] = element;\
    container->last = (container->last + 1) & (container->capacity - 1);\
    container->size++;\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_add(xdeque_##T *container, T element)\
{\
    return xdeque_##T##_add_back(container, element);\
}\
\
enum x_stat xdeque_##T##_add_at(xdeque_##T *container, T element, size_t index)\
{\
    size_t xcapacity;\
    size_t xlast;\
    size_t xfirst;\
    size_t xpos;\
    if (index > container->size) {\
        return X_INDEX_OUT_OF_RANGE_ERR;\
    }\
    if (index == container->size) {\
        return xdeque_##T##_add_back(container, element);\
    }\
    if (container->size >= container->max_size) {\
        return X_MAX_SIZE_ERR;\
    }\
    if (container->capacity == container->size && xdeque_##T##_expand_capacity(container) != X_OK) {\
        return X_ALLOC_ERR;\
    }\
    xcapacity = container->capacity - 1;\
    xlast = container->last & xcapacity;\
    xfirst = container->first & xcapacity;\
    xpos = (container->first + index) & xcapacity;\
    if (index == 0) {\
        return xdeque_##T##_add_front(container, element);\
    }\
    if (index == xcapacity) {\
        return xdeque_##T##_add_back(container, element);\
    }\
    if (index <= (container->size / 2) - 1) {\
        if (xpos < xfirst || xfirst == 0) {\
            const size_t r_move = (xfirst != 0) ? xcapacity - xfirst + 1 : 0;\
            const size_t l_move = xpos;\
            T e_first = container->buffer[0];\
            if (xfirst != 0) {\
                memmove(&(container->buffer[xfirst - 1]),\
                        &(container->buffer[xfirst]),\
                        r_move * sizeof(T));\
            }\
            if (xpos != 0) {\
                memmove(&(container->buffer[0]),\
                        &(container->buffer[1]),\
                        l_move * sizeof(T));\
            }\
            container->buffer[xcapacity] = e_first;\
        } else {\
            memmove(&(container->buffer[xfirst - 1]),\
                    &(container->buffer[xfirst]),\
                    index * sizeof(T));\
        }\
        container->first = (container->first - 1) & xcapacity;\
    } else {\
        if (xpos > xlast || xlast == xcapacity) {\
            T e_last = container->buffer[0];\
            if (xpos != xcapacity) {\
                memmove(&(container->buffer[xpos + 1]),\
                        &(container->buffer[xpos]),\
                        (xcapacity - xpos) * sizeof(T));\
            }\
            if (xlast != xcapacity) {\
                memmove(&(container->buffer[1]),\
                        &(container->buffer[0]),\
                        (xlast + 1) * sizeof(T));\
            }\
            container->buffer[0] = e_last;\
        } else {\
            memmove(&(container->buffer[xpos + 1]),\
                    &(container->buffer[xpos]),\
                    (container->size - index) * sizeof(T));\
        }\
        container->last = (container->last + 1) & xcapacity;\
    }\
    container->buffer[xpos] = element;\
    container->size++;\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_get_at(xdeque_##T *container, int index, T *out)\
{\
    size_t actual_index;\
    if (!out) {\
        return X_OUT_PARAM_NULL_ERR;\
    }\
    if (index >= container->size) {\
        return X_INDEX_OUT_OF_RANGE_ERR;\
    }\
    actual_index = (container->first + index) & (container->capacity - 1);\
    *out = container->buffer[actual_index];\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_get_front(xdeque_##T *container, T *out)\
{\
    if (!out) {\
        return X_OUT_PARAM_NULL_ERR;\
    }\
    if (container->size == 0) {\
        return X_EMPTY_CONTAINER_ERR;\
    }\
    *out = container->buffer[container->first];\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_get_back(xdeque_##T *container, T *out)\
{\
    size_t last;\
    if (!out) {\
        return X_OUT_PARAM_NULL_ERR;\
    }\
    if (container->size == 0) {\
        return X_EMPTY_CONTAINER_ERR;\
    }\
    last = (container->last - 1) & (container->capacity - 1);\
    *out = container->buffer[last];\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_replace_at(xdeque_##T *container, size_t index, T element, T *out)\
{\
    size_t actual_index;\
    if (index >= container->size) {\
        return X_INDEX_OUT_OF_RANGE_ERR;\
    }\
    actual_index = (container->first + index) & (container->capacity - 1);\
    if (out) {\
        *out = container->buffer[actual_index];\
    }\
    container->buffer[actual_index] = element;\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_remove_front(xdeque_##T *container, T *out)\
{\
    T element;\
    if (container->size == 0) {\
        return X_INDEX_OUT_OF_RANGE_ERR;\
    }\
    element = container->buffer[container->first];\
    container->first = (container->first + 1) & (container->capacity - 1);\
    container->size--;\
    if (out) {\
        *out = element;\
    }\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_remove_back(xdeque_##T *container, T *out)\
{\
    size_t last;\
    T element;\
    if (container->size == 0) {\
        return X_INDEX_OUT_OF_RANGE_ERR;\
    }\
    last = (container->last - 1) & (container->capacity - 1);\
    element = container->buffer[last];\
    container->last = last;\
    container->size--;\
    if (out) {\
        *out = element;\
    }\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_remove_at(xdeque_##T *container, size_t index, T *out)\
{\
    size_t xcapacity;\
    size_t xlast;\
    size_t xfirst;\
    size_t xpos;\
   T removed;\
    if (index < 0 || index >= container->size) {\
        return X_INDEX_OUT_OF_RANGE_ERR;\
    }\
    xcapacity = container->capacity - 1;\
    xlast = container->last & xcapacity;\
    xfirst = container->first & xcapacity;\
    xpos = (container->first + index) & xcapacity;\
    removed = container->buffer[index];\
    if (index == 0) {\
        return xdeque_##T##_remove_front(container, out);\
    }\
    if (index == xcapacity) {\
        return xdeque_##T##_remove_back(container, out);\
    }\
    if (index <= (container->size / 2) - 1) {\
        if (xpos < xfirst) {\
            T e = container->buffer[xcapacity];\
            if (xfirst != xcapacity) {\
                memmove(&(container->buffer[xfirst + 1]),\
                        &(container->buffer[xfirst]),\
                        (xcapacity - xfirst) * sizeof(T));\
            }\
            if (xpos != 0) {\
                memmove(&(container->buffer[1]),\
                        &(container->buffer[0]),\
                        xpos * sizeof(T));\
            }\
            container->buffer[0] = e;\
        } else {\
            memmove(&(container->buffer[xfirst + 1]),\
                    &(container->buffer[xfirst]),\
                    index * sizeof(T));\
        }\
        container->first = (container->first + 1) & xcapacity;\
    } else {\
        if (xpos > xlast) {\
            T e = container->buffer[0];\
            if (xpos != xcapacity) {\
                memmove(&(container->buffer[xpos]),\
                        &(container->buffer[xpos + 1]),\
                        (xcapacity - xpos) * sizeof(T));\
            }\
            if (xpos != 0) {\
                memmove(&(container->buffer[1]),\
                        &(container->buffer[0]),\
                        xlast * sizeof(T));\
            }\
            container->buffer[xcapacity] = e;\
        } else {\
            memmove(&(container->buffer[xpos]),\
                    &(container->buffer[xpos + 1]),\
                    (xlast - xpos) * sizeof(T));\
        }\
        container->last = (container->last- 1) & xcapacity;\
    }\
    container->size--;\
    if (out)\
        *out = removed;\
    return X_OK;\
}\
\
enum x_stat xdeque_##T##_clear(xdeque_##T *container)\
{\
    enum x_stat status;\
    while (container->size > 0) {\
        status = xdeque_##T##_remove_at(container, (container->size)-1, NULL);\
        if (status != X_OK) {\
            return status;\
        }\
    }\
    return X_OK;\
}\
\
static void xdeque_##T##_copy_buffer(xdeque_##T const * const container, T *buffer, T (*callback) (T))\
{\
    if (callback == NULL) {\
        if (container->last > container->first) {\
            memcpy(buffer,\
                   &(container->buffer[container->first]),\
                   container->size * sizeof(void*));\
        } else {\
            size_t l = container->last;\
            size_t e = container->capacity - container->first;\
            memcpy(buffer,\
                   &(container->buffer[container->first]),\
                   e * sizeof(void*));\
            memcpy(&(buffer[e]),\
                   container->buffer,\
                   l * sizeof(void*));\
        }\
    } else {\
        size_t i;\
        for (i = 0; i < container->size; i++) {\
            size_t p = (container->first + i) & (container->capacity - 1);\
            buffer[i] = callback(container->buffer[p]);\
        }\
    }\
}\
\
static enum x_stat xdeque_##T##_shrink_to_fit(xdeque_##T *container)\
{\
    size_t new_size;\
    T *new_buffer;\
    if (container->capacity == container->size) {\
        return X_OK;\
    }\
    new_size = x_upper_pow_two(container->size);\
    if (new_size == container->capacity) {\
        return X_OK;\
    }\
    new_buffer = container->memory_alloc(sizeof(T) * new_size);\
    if (!new_buffer) {\
        return X_ALLOC_ERR;\
    }\
    xdeque_##T##_copy_buffer(container, new_buffer, NULL);\
    container->memory_free(container->buffer);\
    container->buffer   = new_buffer;\
    container->first    = 0;\
    container->last     = container->size;\
    container->capacity = new_size;\
    return X_OK;\
}\
\
static enum x_stat xdeque_##T##_expand_capacity(xdeque_##T *container)\
{\
    size_t new_capacity;\
    T *new_buffer;\
    if (container->capacity == X_MAX_POW_TWO) {\
        return X_MAX_CAPACITY_ERR;\
    }\
    new_capacity = container->capacity << 1;\
    new_buffer = container->memory_calloc(new_capacity, sizeof(T));\
    if (!new_buffer) {\
        return X_ALLOC_ERR;\
    }\
    xdeque_##T##_copy_buffer(container, new_buffer, NULL);\
    container->memory_free(container->buffer);\
    container->first      = 0;\
    container->last       = container->size;\
    container->capacity   = new_capacity;\
    container->buffer     = new_buffer;\
    return X_OK;\
}\
\
\

/**

*/
#define xdeque(T) xdeque_##T

/**

*/
#define xdeque_new(T) xdeque_##T##_new

/**

*/
#define xdeque_new_max_size(T) xdeque_##T##_new_max_size

/**

*/
#define xdeque_new_config(T) xdeque_##T##_new_config

/**

*/
#define xdeque_add_front(T) xdeque_##T##_add_front

/**

*/
#define xdeque_add_back(T) xdeque_##T##_add_back

/**

*/
#define xdeque_add(T) xdeque_##T##_add

/**

*/
#define xdeque_add_at(T) xdeque_##T##_add_at

/**

*/
#define xdeque_get_at(T) xdeque_##T##_get_at

/**

*/
#define xdeque_get_front(T) xdeque_##T##_get_front

/**

*/
#define xdeque_get_back(T) xdeque_##T##_get_back

/**

*/
#define xdeque_replace_at(T) xdeque_##T##_replace_at

/**

*/
#define xdeque_remove_front(T) xdeque_##T##_remove_front

/**

*/
#define xdeque_remove_back(T) xdeque_##T##_remove_back

/**

*/
#define xdeque_remove_at(T) xdeque_##T##_remove_at

/**

*/
#define xdeque_clear(T) xdeque_##T##_clear

/**

*/
#define xdeque_shrink_to_fit(T) xdeque_##T##_shrink_to_fit

/**

*/
#define xdeque_destroy(container) { \
        container->memory_free(container->buffer); \
        container->memory_free(container->iter); \
        container->memory_free(container); \
    }

/*

*/
#define xdeque_capacity xcapacity

/*

*/
#define xdeque_size xsize

/*

*/
#define xdeque_max_size xmax_size

/*

*/
#define xdeque_is_empty xis_empty


#ifdef __cplusplus
}
#endif

#endif