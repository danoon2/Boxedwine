/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef BOXEDWINE_ES
#include "SDL_opengles2.h"
#define GLclampd double
#define GLdouble double
#include "../glcommon.h"
#include "esdisplaylist.h"
#include "glshim.h"
#include "kalloc.h"

struct ListOp* allocListOp(struct DisplayList* list) {
    struct ListOp* result = kalloc(sizeof(struct ListOp));
    if (!list->head) {
        list->head = result;		
    } else {
        list->tail->next = result;		
    }
    list->tail = result;
    return result;
}

void list_glClearColor(struct ListOp* op) {
    glClearColor(op->a1.f, op->a2.f, op->a3.f, op->a4.f);
}

void list_glClear(struct ListOp* op) {
    glClear(op->a1.i);
}

void list_glClearDepth(struct ListOp* op) {
    glClearDepthf(op->a1.f);
}

void list_glClearStencil(struct ListOp* op) {
    glClearStencil(op->a1.i);
}

void list_glColorMask(struct ListOp* op) {
    glColorMask(op->a1.i, op->a2.i, op->a3.i, op->a4.i);
}

void list_glBlendFunc(struct ListOp* op) {
    glBlendFunc(op->a1.i, op->a2.i);
}

void list_glCullFace(struct ListOp* op) {
    glCullFace(op->a1.i);
}

void list_glFrontFace(struct ListOp* op) {
    glFrontFace(op->a1.i);
}

void list_glLineWidth(struct ListOp* op) {
    glLineWidth(op->a1.f);
}

void list_glLineStipple(struct ListOp* op) {
    glLineStipple(op->a1.i, op->a2.i);
}

void list_glPolygonOffset(struct ListOp* op) {
    glPolygonOffset(op->a1.f, op->a2.f);
}

void list_glScissor(struct ListOp* op) {
    glScissor(op->a1.i, op->a2.i, op->a3.i, op->a4.i);
}

void list_shim_glEnable(struct ListOp* op) {
    shim_glEnable(op->a1.i);
}

void list_shim_glDisable(struct ListOp* op) {
    shim_glDisable(op->a1.i);
}
#endif