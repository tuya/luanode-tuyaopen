
#ifndef LUAT_TIMER_H
#define LUAT_TIMER_H

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lua_msgbus.h"
#include "tal_sw_timer.h"

typedef struct
{
    void* os_timer;
    size_t id;
    size_t timeout;
    size_t type;
    TIMER_TYPE  repeat;
    LUA_MSG_HANDLE func;
}LUA_TIMER_T;


int lua_timer_start(LUA_TIMER_T* timer,BOOL_T new);

int lua_timer_stop(LUA_TIMER_T* timer);

int lua_timer_trigger(LUA_TIMER_T* timer);

int lua_timer_delete(LUA_TIMER_T* timer);

LUA_TIMER_T* lua_timer_get(int timer_id);

#endif

