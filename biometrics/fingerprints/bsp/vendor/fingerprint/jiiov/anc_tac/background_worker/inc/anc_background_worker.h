#ifndef ANC_BACKGROUND_WORKER_H
#define ANC_BACKGROUND_WORKER_H

#include <pthread.h>
#include "anc_error.h"
#include "anc_memory_wrapper.h"

typedef struct {
    void *data;
    void (*Excute)(void*);
    void (*Free)(void*);
}AncBGTask;


typedef struct AncBGTaskNode {
    AncBGTask task;
    struct AncBGTaskNode *next;
}AncBGTaskNode_t;

typedef struct {
    ANC_RETURN_TYPE (*Init)();
    ANC_RETURN_TYPE (*Deinit)();
    ANC_RETURN_TYPE (*PushTask)(AncBGTask task);
}AncBGWorker;

#endif
