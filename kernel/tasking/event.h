#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <utils/common.h>
#include <utils/spinlock.h>

typedef uint32_t event_t;

bool event_array_await_timeout(event_t **events, bool *outEvents, size_t count, size_t timeout);
bool event_await_timeout(event_t *event, size_t timeout);
bool event_await(event_t *event);
void event_notify(event_t *event);
void event_reset(event_t *event);