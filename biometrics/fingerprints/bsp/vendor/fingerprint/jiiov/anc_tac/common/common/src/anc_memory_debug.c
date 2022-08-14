#define LOG_TAG "[ANC_COMMON][MemoryDebug]"

#include "anc_memory_debug.h"

#include "anc_log.h"
#include "anc_error.h"
#include "anc_memory.h"

// malloc/free use list record memory info
#define FUNC_NAME_MAXLEN 64
#define ANC_HASHMAP_SIZE 79

// memory list node
typedef struct {
    void *p_mem_addr;                      // memory addr
    uint32_t mem_size;                     // memory size
    char function_name[FUNC_NAME_MAXLEN];  // the function name call malloc
    uint16_t file_line_num;                // code line
} AncMemBlock;

typedef struct _AncMemListRecord {
    AncMemBlock mem_block;
    struct _AncMemListRecord *p_next;
} AncMemListRecord;

// function call malloc count map node define
typedef struct {
    char function_name[FUNC_NAME_MAXLEN];  // the function name call malloc
    uint16_t func_malloc_cnt;              // function call malloc count
} AncCallMallocInfo;

typedef struct _AncMemHashNode {
    AncCallMallocInfo mem_block;
    struct _AncMemHashNode *p_next;
} AncMemHashNode;

// memory malloc frequency hashmap
typedef struct {
    uint32_t mem_table_size;      // hashTable size
    AncMemHashNode *p_mem_table;  // hash table
} AncMemHashMap;

typedef struct {
    uint32_t mem_peak_size;          // current memory peak size
    uint32_t mem_total_size;         // current memory used size
    AncMemListRecord *p_mem_record;  // memory record
    AncMemHashMap *p_mem_map;        // memory hashmap, no need Destory
} AncMemDebug;

static AncMemDebug *gp_anc_mem_debug = NULL;

static AncMemHashMap *AncMemCreatHashMap() {
    // function call malloc frequency
    AncMemHashMap *p_mem_hashmap =
        (AncMemHashMap *)AncPlatformMalloc(sizeof(AncMemHashMap));
    if (NULL == p_mem_hashmap) {
        ANC_LOGE("mem hashmap malloc failed");
        return NULL;
    }

    AncPlatformMemset(p_mem_hashmap, 0, sizeof(AncMemHashMap));

    p_mem_hashmap->mem_table_size = ANC_HASHMAP_SIZE;

    p_mem_hashmap->p_mem_table = (AncMemHashNode *)AncPlatformMalloc(
        p_mem_hashmap->mem_table_size * sizeof(AncMemHashNode));
    if (NULL == p_mem_hashmap->p_mem_table) {
        AncPlatformFree(p_mem_hashmap);
        p_mem_hashmap = NULL;
        ANC_LOGE("memory table malloc failed");
        return NULL;
    }

    uint32_t i = 0;
    for (i = 0; i < p_mem_hashmap->mem_table_size; i++) {
        AncPlatformMemset(&(p_mem_hashmap->p_mem_table[i].mem_block), 0,
                          sizeof(AncCallMallocInfo));
        p_mem_hashmap->p_mem_table[i].p_next = NULL;
    }

    return p_mem_hashmap;
}

// BKDR Hash Function from internet
static unsigned int BKDRHash(const char *p_str) {
    unsigned int seed = 131;  // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*p_str) {
        hash = hash * seed + (*p_str++);
    }
    return (hash & 0x7FFFFFFF);
}

static ANC_RETURN_TYPE AncHashMapPut(AncMemHashMap *p_hashmap,
                                     AncMemBlock *p_mem_block) {
    if ((NULL == p_hashmap) || (NULL == p_mem_block)) {
        ANC_LOGE("memory map or put block is null");
        return ANC_FAIL;
    }
    uint32_t index =
        BKDRHash(p_mem_block->function_name) % (p_hashmap->mem_table_size);
    AncMemHashNode *p_curr_list = &(p_hashmap->p_mem_table[index]);

    if (p_curr_list->mem_block.func_malloc_cnt == 0) {  // no conflict
        AncPlatformMemcpy(&(p_curr_list->mem_block.function_name),
                          p_mem_block->function_name,
                          strlen(p_mem_block->function_name));
        p_curr_list->mem_block.func_malloc_cnt++;
    } else {  // conflict, find the same function_name in list node,increase cnt
        while (p_curr_list->p_next != NULL) {
            if (strcmp(p_curr_list->mem_block.function_name,
                       p_mem_block->function_name) == 0) {
                p_curr_list->mem_block.func_malloc_cnt++;
                break;
            }
            p_curr_list = p_curr_list->p_next;
        }
        if (NULL == p_curr_list->p_next) {
            if (strcmp(p_curr_list->mem_block.function_name,
                       p_mem_block->function_name) == 0) {
                p_curr_list->mem_block.func_malloc_cnt++;
            } else {  // conflict, no find the same function_name in list node
                      // insert it
                AncMemHashNode *p_mem_node =
                    (AncMemHashNode *)AncPlatformMalloc(sizeof(AncMemHashNode));
                if (p_mem_node == NULL) {
                    ANC_LOGE("mem malloc failed");
                    return ANC_FAIL;
                }
                AncPlatformMemcpy(&(p_mem_node->mem_block.function_name),
                                  p_mem_block->function_name,
                                  strlen(p_mem_block->function_name));
                p_mem_node->mem_block.func_malloc_cnt = 1;
                p_mem_node->p_next = NULL;
                p_curr_list->p_next = p_mem_node;
            }
        }
    }
    return ANC_OK;
}

#if 0 // for : unused function 'AncHashMapGet' [-Werror,-Wunused-function]
static ANC_RETURN_TYPE AncHashMapGet(const char *p_key, uint16_t *p_value) {
    if (NULL == p_key) {
        ANC_LOGE("function name is null");
        return ANC_FAIL;
    }
    AncMemHashMap *p_hashmap = gp_anc_mem_debug->p_mem_map;
    uint32_t index = BKDRHash(p_key) % (p_hashmap->mem_table_size);
    AncMemHashNode *p_curr_list = &(p_hashmap->p_mem_table[index]);

    while (p_curr_list != NULL) {
        if (strcmp(p_curr_list->mem_block.function_name, p_key) == 0) {
            *p_value = p_curr_list->mem_block.func_malloc_cnt;
            return ANC_OK;
        } else {
            p_curr_list = p_curr_list->p_next;
        }
    }

    return ANC_FAIL;
}
#endif

static ANC_RETURN_TYPE AncMemDebugInit() {
    gp_anc_mem_debug = (AncMemDebug *)AncPlatformMalloc(sizeof(AncMemDebug));
    if (NULL == gp_anc_mem_debug) {
        ANC_LOGE("anc memory debug malloc failed!");
        return ANC_FAIL;
    }

    AncPlatformMemset(gp_anc_mem_debug, 0, sizeof(AncMemDebug));
    gp_anc_mem_debug->p_mem_record = NULL;  // no header list

    gp_anc_mem_debug->p_mem_map = AncMemCreatHashMap();
    if (NULL == gp_anc_mem_debug->p_mem_map) {
        AncPlatformFree(gp_anc_mem_debug);
        gp_anc_mem_debug = NULL;
        ANC_LOGE("anc memory map create failed");
        return ANC_FAIL;
    }
    return ANC_OK;
}

static ANC_RETURN_TYPE AncMemListInsert(AncMemListRecord **p_list,
                                        AncMemBlock *p_mem_info) {
    // todo realloc
    AncMemListRecord *p_record =
        (AncMemListRecord *)AncPlatformMalloc(sizeof(AncMemListRecord));
    if (NULL == p_record) {
        ANC_LOGE("insert node malloc failed");
        return ANC_FAIL;
    }
    AncPlatformMemcpy(&(p_record->mem_block), p_mem_info, sizeof(AncMemBlock));
    p_record->p_next = *p_list;
    *p_list = p_record;
    return ANC_OK;
}

void AncRegisterAllocation(void *p_addr, size_t mem_size,
                           const char *p_func_name, int code_line) {
    if (NULL == gp_anc_mem_debug) {
        if (ANC_OK != AncMemDebugInit()) {
            ANC_LOGE("anc memory debug init failed");
            return;
        }
    }

    if (gp_anc_mem_debug != NULL) {
        gp_anc_mem_debug->mem_total_size += mem_size;
        if (gp_anc_mem_debug->mem_total_size > gp_anc_mem_debug->mem_peak_size) {
            gp_anc_mem_debug->mem_peak_size = gp_anc_mem_debug->mem_total_size;
        }
        ANC_LOGD(
            "###### Anc Memory PEAK = (%d), current add size = (%zu), total "
            "size "
            "=(%d),function name: %s, line: %d",
            gp_anc_mem_debug->mem_peak_size, mem_size,
            gp_anc_mem_debug->mem_total_size, p_func_name, code_line);

        AncMemBlock *p_mem_info = (AncMemBlock *)AncPlatformMalloc(sizeof(AncMemBlock));
        AncPlatformMemset(p_mem_info, 0, sizeof(AncMemBlock));
        p_mem_info->p_mem_addr = p_addr;
        p_mem_info->mem_size = (uint32_t)mem_size;
        p_mem_info->file_line_num = (uint16_t)code_line;
        size_t name_len = strlen(p_func_name) + 1;
        AncPlatformMemcpy(
            p_mem_info->function_name, p_func_name,
            (name_len > FUNC_NAME_MAXLEN) ? FUNC_NAME_MAXLEN : name_len);

        if (ANC_OK != AncMemListInsert(&(gp_anc_mem_debug->p_mem_record), p_mem_info)) {
              AncPlatformFree(p_mem_info);
            ANC_LOGE("anc memory insert failed");
            return;
        }

        // insert the memory info into map
        if (ANC_OK != AncHashMapPut(gp_anc_mem_debug->p_mem_map, p_mem_info)) {
            AncPlatformFree(p_mem_info);
            ANC_LOGE("anc memory map insert failed");
            return;
        }
        AncPlatformFree(p_mem_info);
    }
}

static AncMemListRecord *AncListFindPreNode(void *p_addr,
                                            AncMemListRecord *p_list) {
    AncMemListRecord *p = p_list;
    while ((p != NULL) && (p->p_next != NULL) &&
           (p->p_next->mem_block.p_mem_addr != p_addr)) {
        p = p->p_next;
    }
    return p;
}

void AncUnRegisterAllocation(void *p_addr) {
    if (NULL == p_addr) {
        ANC_LOGE("anc unregister p_addr is null");
        return;
    }

    AncMemListRecord *p_record_tmp = gp_anc_mem_debug->p_mem_record;
    if (NULL == p_record_tmp) {
        ANC_LOGE("anc unregister p_record_tmp is null");
        return;
    }

    // handle the first node
    if (p_addr == p_record_tmp->mem_block.p_mem_addr) {
        gp_anc_mem_debug->p_mem_record = p_record_tmp->p_next;
        gp_anc_mem_debug->mem_total_size -= p_record_tmp->mem_block.mem_size;
        AncPlatformFree(p_record_tmp);
        p_record_tmp = NULL;
    } else {
        // handle the left node
        AncMemListRecord *p =
            AncListFindPreNode(p_addr, gp_anc_mem_debug->p_mem_record);
        if ((p != NULL) && (p->p_next != NULL)) {
            AncMemListRecord *p_tmp = p->p_next;
            p->p_next = p_tmp->p_next;
            gp_anc_mem_debug->mem_total_size -= p_tmp->mem_block.mem_size;
            AncPlatformFree(p_tmp);
            p_tmp = NULL;
        } else {  // no find the p_addr
            ANC_LOGE("anc free illegal address %p", p_addr);
        }
    }

    // free memory record when total memory size is 0
    if (gp_anc_mem_debug->mem_total_size == 0) {
        AncPlatformFree(gp_anc_mem_debug->p_mem_record);
        gp_anc_mem_debug->p_mem_record = NULL;
    }
}

static void AncMemoryBlockPrint() {
    ANC_LOGD("===============anc memory block information===============");
    if ((gp_anc_mem_debug != NULL) && (gp_anc_mem_debug->p_mem_record != NULL)) {
        ANC_LOGD("### Memory Peak is %d, Current Total Memory is %d",
                  gp_anc_mem_debug->mem_peak_size,
                  gp_anc_mem_debug->mem_total_size);
        AncMemListRecord *p = gp_anc_mem_debug->p_mem_record;
        while (p != NULL) {
            ANC_LOGD(
                "p_addr = %p, mem_size = %d, function name: %s, line = %d",
                p->mem_block.p_mem_addr, p->mem_block.mem_size,
                p->mem_block.function_name, p->mem_block.file_line_num);
            p = p->p_next;
        }
    }
    ANC_LOGD("===============anc memory block information End===============");
}

static void AncMallocCallCntPrint() {
    ANC_LOGD("===============anc malloc call count information============");

    uint32_t i = 0;

    if ((gp_anc_mem_debug != NULL) && (gp_anc_mem_debug->p_mem_map != NULL)) {
        AncMemHashMap *p_hashmap = gp_anc_mem_debug->p_mem_map;
        AncMemHashNode *p;
        while (i < p_hashmap->mem_table_size) {
            p = &(p_hashmap->p_mem_table[i]);
            while (p != NULL) {
                if (p->mem_block.func_malloc_cnt != 0) {
                    ANC_LOGD("%s call malloc times :%d",
                              p->mem_block.function_name,
                              p->mem_block.func_malloc_cnt);
                }
                p = p->p_next;
            }
            i++;
        }
    }
    ANC_LOGD("===============anc malloc call count information End============");
}

static void AncMemoryMapDestory() {
    if ((NULL == gp_anc_mem_debug) || (NULL == gp_anc_mem_debug->p_mem_map)) {
        ANC_LOGE("anc memory map destory null");
        return;
    }
    AncMemHashMap *p_hashmap = gp_anc_mem_debug->p_mem_map;
    AncMemHashNode *curr_node;
    uint32_t i = 0;
    while (i < p_hashmap->mem_table_size) {
        curr_node = &(p_hashmap->p_mem_table[i]);  // currrnt node addr
        AncMemHashNode *p_node = curr_node->p_next;
        while (p_node != NULL) {
            curr_node->p_next = p_node->p_next;
            AncPlatformFree(p_node);
            p_node = curr_node->p_next;
        }
        i++;
    }
    AncPlatformFree(p_hashmap->p_mem_table);
    p_hashmap->p_mem_table = NULL;
    AncPlatformFree(p_hashmap);
    gp_anc_mem_debug->p_mem_map = NULL;
}

static void AncMemoryRecordDestory() {
    if (NULL == gp_anc_mem_debug) {
        ANC_LOGE("anc memory record destory, invalid param, memory debug gp_anc_mem_debug is NULL");
        return;
    }
    if (NULL == gp_anc_mem_debug->p_mem_record) {
        ANC_LOGE("anc memory record destory, invalid param, memory record p_mem_record is NULL");
        return;
    }
    AncMemListRecord *p_record_node = gp_anc_mem_debug->p_mem_record;
    while (p_record_node != NULL) {
        AncMemListRecord *p_temp = p_record_node->p_next;
        AncPlatformFree(p_record_node);
        p_record_node = p_temp;
    }
    gp_anc_mem_debug->p_mem_record = NULL;
}

void AncMemDebugRelease() {
    AncMemoryRecordDestory();
    AncMemoryMapDestory();
    if (gp_anc_mem_debug != NULL) {
        AncPlatformFree(gp_anc_mem_debug);
        gp_anc_mem_debug = NULL;
    }
    ANC_LOGD("===============anc memory debug release End===============");
}

ANC_RETURN_TYPE AncMemoryCheck() {
    AncMallocCallCntPrint();
    AncMemoryBlockPrint();
    if ((gp_anc_mem_debug == NULL) || (gp_anc_mem_debug->p_mem_record == NULL)) {
        return ANC_OK;
    } else {
        return ANC_FAIL_MEMORY;
    }
}
