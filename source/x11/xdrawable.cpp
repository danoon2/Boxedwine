#include "boxedwine.h"
#include "x11.h"

XDrawable::XDrawable(U32 width, U32 height, U32 depth) : id(XServer::getNextId()), width(width), height(height), depth(depth) {
}