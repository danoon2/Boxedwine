#include "boxedwine.h"
#include "xdrawable.h"
#include "displaydata.h"

XDrawable::XDrawable(U32 width, U32 height, U32 depth) : id(DisplayData::getNextId()), width(width), height(height), depth(depth) {
}