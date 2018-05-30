#ifndef __ESDISPLAYLIST_H__
#define __ESDISPLAYLIST_H__

struct DisplayList {
	int index;
	int mode;
	struct ListOp* head;
	struct ListOp* tail;
};

struct ListOpArg {
	union {
		GLint i;
		GLfloat f;
	};
};

typedef void (*ListOpCallback)(struct ListOp* op);

struct ListOp {
	ListOpCallback op;
	struct ListOpArg a1;
	struct ListOpArg a2;
	struct ListOpArg a3;
	struct ListOpArg a4;
	struct ListOp* next;
};

struct ListOp* allocListOp(struct DisplayList* list);

void list_glClearColor(struct ListOp* op);
void list_glClear(struct ListOp* op);
void list_glClearDepth(struct ListOp* op);
void list_glClearStencil(struct ListOp* op);
void list_glColorMask(struct ListOp* op);
void list_glBlendFunc(struct ListOp* op);
void list_glCullFace(struct ListOp* op);
void list_glFrontFace(struct ListOp* op);
void list_glLineWidth(struct ListOp* op);
void list_glLineStipple(struct ListOp* op);
void list_glPolygonOffset(struct ListOp* op);
void list_glScissor(struct ListOp* op);
void list_shim_glEnable(struct ListOp* op);
void list_shim_glDisable(struct ListOp* op);

#endif