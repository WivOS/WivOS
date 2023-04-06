#include "event.h"

#include "scheduler.h"
#include <devices/timer.h>
#include <cpu/cpu.h>

bool events_await(event_t **event, bool *outEvents, size_t count) {
    bool wake = false;

    for(size_t i = 0; i < count; i++) {
        if(locked_read(event_t, event[i])) {
            wake = true;
            locked_dec(event[i]);
            if(outEvents) outEvents[i] = true;
        }
    }
    if(wake) return true;

    spinlock_lock(&SchedulerLock);
    volatile thread_t *currentThread = ActiveTasks[CPULocals[CurrentCPU].currentTaskID];
    currentThread->event_count = count;
    currentThread->out_event_pointer = outEvents;
    spinlock_unlock(&SchedulerLock);

    locked_write(event_t **, &currentThread->event_pointer, event);
    yield();
    if(locked_read(event_t, &currentThread->event_abort))
        return false;

    return true;
}

extern volatile size_t TimerCounter;
bool event_array_await_timeout(event_t **events, bool *outEvents, size_t count, size_t timeout) {
    uint64_t target = (TimerCounter  + (timeout * (TIMER_FREQ / 1000))) + 1;

    spinlock_lock(&SchedulerLock);
    volatile thread_t *currentThread = ActiveTasks[CPULocals[CurrentCPU].currentTaskID];
    currentThread->event_timeout = target;
    spinlock_unlock(&SchedulerLock);

    bool ret = events_await(events, outEvents, count);
    if(!ret) return false;
    else return currentThread->event_timeout != 0;
}

bool event_await_timeout(event_t *event, size_t timeout) {
    event_t *events[1] = {event};
    bool eventsOut[1] = {false};
    return event_array_await_timeout(events, eventsOut, 1, timeout);
}

bool event_await(event_t *event) {
    event_t *events[1] = {event};
    bool eventsOut[1] = {false};
    return events_await(events, eventsOut, 1);
}

void event_notify(event_t *event) {
    locked_inc(event);
}

void event_reset(event_t *event) { //Reseted the event count
    while(locked_read(event_t, event)) {
        locked_dec(event);
    }
}