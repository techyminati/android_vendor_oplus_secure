#ifndef __OBJECT_DEFINITION_H__
#define __OBJECT_DEFINITION_H__

#include <stdint.h>

#define MAKE_TAG_CONSTANT(A, B, C, D) (((A) << 24) | ((B) << 16) | ((C) << 8) | (D))
#define RBS_OBJ1_TAG MAKE_TAG_CONSTANT('O', 'B', 'J', '1')
#define OBJ_NAME_SIZE 16

#define RBSOBJ_ID_OBJARRAY 101
#define RBSOBJ_ID_OBJARRAY_REQ 102

#define RBSOBJ_ID_IMAGE 151

#ifdef _WINDOWS
#define PACKED_STRUCT(__Declaration__) __pragma(pack(push, 1)) typedef struct __Declaration__ __pragma(pack(pop))
#else
#define PACKED_STRUCT(__Declaration__) typedef struct __Declaration__ __attribute__((packed))
#endif

PACKED_STRUCT( {
	uint32_t tag;  // RBS_OBJ1_TAG
	uint32_t id;   // Uniqure id
	uint8_t name[OBJ_NAME_SIZE];
	uint16_t version_major;
	uint16_t version_minor;
	uint32_t payload_size;
	uint16_t header_size;
}) rbs_obj_desc_t;

PACKED_STRUCT( {
	rbs_obj_desc_t obj_desc;
	uint16_t index_start;
	uint16_t count;
	uint16_t has_more_object;
}) rbs_obj_array_t;

#define OBJ_DESC_init(obj_desc, the_id, the_name, major, minor, the_header_size)                               \
	obj_desc->tag = RBS_OBJ1_TAG;                                                                          \
	obj_desc->id = the_id;                                                                                 \
	memcpy(obj_desc->name, the_name, sizeof(the_name) > OBJ_NAME_SIZE ? OBJ_NAME_SIZE : sizeof(the_name)); \
	obj_desc->name[OBJ_NAME_SIZE - 1] = '\0';                                                              \
	obj_desc->version_major = major;                                                                       \
	obj_desc->version_minor = minor;                                                                       \
	obj_desc->payload_size = 0;                                                                            \
	obj_desc->header_size = the_header_size;

#define RBSOBJ_init_OBJARRAY_v1_0(objarray, idx_start, tcount)                           \
	OBJ_DESC_init((&(objarray->obj_desc)), RBSOBJ_ID_OBJARRAY, "OBJARRAY", 1, 0, 0); \
	objarray->obj_desc.header_size = sizeof(*objarray) - sizeof(objarray->obj_desc); \
	objarray->obj_desc.payload_size = 0;                                             \
	objarray->index_start = idx_start;                                               \
	objarray->count = tcount;                                                        \
	objarray->has_more_object = 0;

#define RBSOBJ_get_obj_total_size(obj) \
	obj->obj_desc.payload_size + sizeof(*obj)

#define RBSOBJ_get_payload_pointer(obj) \
	(((uint8_t*)obj) + sizeof(obj->obj_desc) + obj->obj_desc.header_size)

#define RBSOBJ_get_payload_size(obj) \
	obj->obj_desc.payload_size

#define RBSOBJ_get_objarray_obj_id(obj) \
	((rbs_obj_desc_t*)RBSOBJ_get_payload_pointer(obj))->id

#define RBSOBJ_set_objarray_payload_size(objarray, the_obj) \
	objarray->obj_desc.payload_size = RBSOBJ_get_obj_total_size(the_obj) * objarray->count;


typedef enum array_reset_mode {
	ARRAY_REQ_RESET_NEVER,
	ARRAY_REQ_RESET_AUTO,
	ARRAY_REQ_RESET_ALWAYS
} array_reset_mode_t;

typedef struct {
	rbs_obj_desc_t obj_desc;
	uint16_t index_start;
	uint16_t count;
	uint16_t reset_mode;
	uint32_t req_obj_id;
	uint16_t param1;
	uint16_t param2;
} rbs_obj_array_request_t;

#define RBSOBJ_set_ARRAY_REQ_v1_0(arrayreq, idx_start, tcount, req_objid)                \
	OBJ_DESC_init((&(arrayreq->obj_desc)), RBSOBJ_ID_OBJARRAY_REQ, "OBJARRAY_REQ", 1, 0, 0); \
	arrayreq->obj_desc.header_size = sizeof(*arrayreq) - sizeof(arrayreq->obj_desc);         \
	arrayreq->obj_desc.payload_size = 0;                                                     \
	arrayreq->index_start = idx_start;                                                       \
	arrayreq->count = tcount;                                                                \
	arrayreq->reset_mode = ARRAY_REQ_RESET_NEVER;                                            \
	arrayreq->req_obj_id = req_objid;

#endif