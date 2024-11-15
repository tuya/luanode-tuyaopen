
#ifndef LUAT_TIMER_H
#define LUAT_TIMER_H

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lua_msgbus.h"
#include "tal_sw_timer.h"

typedef struct
{
    TIMER_ID os_timer;
    size_t timeout;
    size_t type;

    lua_State *L;
    int func_ref;
    int arg_ref;
}LUA_TIMER_T;

#endif

