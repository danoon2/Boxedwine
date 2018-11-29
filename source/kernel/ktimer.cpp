#include "boxedwine.h"

KTimer::~KTimer() {
    if (this->active) {
        removeTimer(this);
    }
}