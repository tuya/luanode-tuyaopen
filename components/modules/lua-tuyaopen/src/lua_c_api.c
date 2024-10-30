#include "lua_c_api.h"
#include "tuya_list.h"
#include "tal_memory.h"
#include "tal_log.h"
typedef struct c_api_map_manager{
    P_LIST_HEAD list_head;
    int api_count;
} C_API_MAP_MANAGER_T;


static C_API_MAP_MANAGER_T s_api_list;

static char error_msg[256] = {0};

#define CAPI_INIT_LIST_HEAD(ptr) do { \
(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

#define CAPI_LIST_FOR_EACH_SAFE(p, n, head) \
for (p = (head)->next; n = p->next, p != (head); p = n)

#define CAPI_LIST_ENTRY(ptr, type, member) \
((type *)((char *)(ptr)-(size_t)(&((type *)0)->member)))

static void __list_add(const P_LIST_HEAD pNew, const P_LIST_HEAD pPrev,\
                       const P_LIST_HEAD pNext)
{
    pNext->prev = pNew;
    pNew->next = pNext;
    pNew->prev = pPrev;
    pPrev->next = pNew;
}

static void __list_del(const P_LIST_HEAD pPrev, const P_LIST_HEAD pNext)
{
    pNext->prev = pPrev;
    pPrev->next = pNext;
}

static void capi_list_add_tail(const P_LIST_HEAD pNew, const P_LIST_HEAD pHead)
{
    __list_add(pNew, pHead->prev, pHead);
}

void capi_list_del(const P_LIST_HEAD pEntry)
{
    __list_del(pEntry->prev, pEntry->next);
}

void lua_capi_registor_api_map(const char *name, void *fun)
{
    CAPI_DECS_T *capi = NULL;
    static BOOL_T bInited = FALSE;
    if (!bInited) {
        memset(&s_api_list,0,sizeof(s_api_list));
        if (s_api_list.list_head == NULL) {
            s_api_list.list_head = tal_malloc(sizeof(LIST_HEAD));
            CAPI_INIT_LIST_HEAD(s_api_list.list_head);
        }
        bInited = TRUE;
    }

    capi = tal_malloc(sizeof(CAPI_DECS_T));
    if (capi) {
        memset(capi->name,0,sizeof(capi->name));

        strncpy(capi->name,name,(sizeof(capi->name)-1));

        capi->api_handle = fun;

        capi_list_add_tail(&capi->list, s_api_list.list_head);

        s_api_list.api_count ++;
    }
}

void *lua_capi_dlsym(void *handle,const char *symbol)
{
    bool bfound = false;
    struct tuya_list_head *p = NULL;
    struct tuya_list_head *n = NULL;
    CAPI_DECS_T *api_item = NULL;
    CAPI_LIST_FOR_EACH_SAFE(p, n, s_api_list.list_head) {
        api_item = CAPI_LIST_ENTRY(p, CAPI_DECS_T, list);
        if (api_item) {
            if (!strcasecmp(api_item->name,symbol)) {
                bfound = true;
                break;
            }
        }
    }
    if (!bfound) {
        snprintf(error_msg,(sizeof(error_msg)-1),"not found api:%s",symbol);
        return NULL;
    }
    return api_item->api_handle;
}


void *lua_capi_dlopen(void)
{
    return &s_api_list;
}

char* lua_cpai_dlerror(void)
{
    return error_msg;
}