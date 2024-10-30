#ifndef LUA_C_API_H
#define LUA_C_API_H
#include <stddef.h>
#include "tuya_cloud_types.h"
#include "tuya_list.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT    ((void *)0)
#endif

typedef struct {
    LIST_HEAD list;
    char name[64];
    void *api_handle;
}CAPI_DECS_T;

void lua_capi_registor_api_map(const char *name, void *fun);

void *lua_capi_dlopen(void);

void *lua_capi_dlsym(void *handle,const char *symbol);

char* lua_cpai_dlerror(void);
#ifdef __cplusplus
}
#endif
#endif /* LUA_C_API */
