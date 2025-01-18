#pragma once

#ifndef _USERMODE_

#include <utils/common.h>
#include <modules/modules.h>
#include <devices/pci.h>

#include <tasking/scheduler.h>

#else

typedef size_t kpid_t;
typedef size_t ktid_t;

#endif

