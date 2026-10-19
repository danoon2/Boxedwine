/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __X11_FLUSH_H__
#define __X11_FLUSH_H__

#include <atomic>
#include "../../tools/x11/X11_def.h"

// Wine flushes both X displays after polling input. That alone must not force
// another host frame. Keep all other requests conservative, including GC work:
// Bang! Bang! flushes while setting up drawing, before Wine uploads its pixels.
class X11FlushState {
public:
    void recordRequest(unsigned int request) {
        switch (request) {
        case X11_FLUSH:
        case X11_FIND_CONTEXT:
        case X11_GET_EVENT:
        case X11_FILTER_EVENT:
        case X11_LOOKUP_STRING:
        case X11_QUERY_POINTER:
        case X11_LOCK_EVENTS:
        case X11_UNLOCK_EVENTS:
        case X11_REMOVE_EVENT:
        case X11_GET_EVENT_DATA:
        case X11_FREE_EVENT_DATA:
        case X11_PUT_BACK_EVENT:
            break;
        default:
            pendingRequests.store(true);
            break;
        }
    }

    bool takePendingRequests() {
        return pendingRequests.exchange(false);
    }

private:
    std::atomic<bool> pendingRequests{true};
};

#endif
