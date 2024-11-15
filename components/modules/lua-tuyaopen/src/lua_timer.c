
#include "tal_sw_timer.h"
#include "lua_timer.h"
#include "lua_msgbus.h"
#include "tal_log.h"
#include "tal_api.h"

#define TIMER_MT "timer"

static int __timer_callback(TIMER_ID os_timer, void *arg)
{
    LUA_TIMER_T *timer = (LUA_TIMER_T*)arg;

    lua_State *L = timer->L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, timer->func_ref);
    if (timer->arg_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, timer->arg_ref);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_pop(L, 1);
        }
    } else {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            lua_pop(L, 1);
        }
    }

    return OPRT_OK;
}

int lua_timer_stop(lua_State *L)
{
    LUA_TIMER_T *timer = luaL_checkudata(L, 1, TIMER_MT);

    if (OPRT_OK != tal_sw_timer_stop(timer->os_timer)) {
        lua_pushboolean(L, 0);
    } else {
        lua_pushboolean(L, 1);
    }
    return 1;
}

int lua_timer_delete(lua_State *L)
{
    LUA_TIMER_T *timer = luaL_checkudata(L, 1, TIMER_MT);

    if (OPRT_OK != tal_sw_timer_delete(timer->os_timer)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    // release ref
    luaL_unref(L, LUA_REGISTRYINDEX, timer->func_ref);
    if (timer->arg_ref != LUA_NOREF) 
        luaL_unref(L, LUA_REGISTRYINDEX, timer->arg_ref);

    lua_pushboolean(L, 1);    
    return 1;
}

int lua_timer_trigger(lua_State *L)
{
    LUA_TIMER_T *timer = luaL_checkudata(L, 1, TIMER_MT);

    if (OPRT_OK != tal_sw_timer_trigger(timer->os_timer)) {
        lua_pushboolean(L, 0);
    } else {
        lua_pushboolean(L, 1);
    }

    return 0;
}

static int lua_timer_new(lua_State *L) 
{
    tal_sw_timer_init();

    int timeout = luaL_checkinteger(L, 1);  // timeout in ms
    int type = luaL_checkinteger(L, 2);

    // function
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    // argument support nil
    int arg_ref = LUA_NOREF;
    if (!lua_isnoneornil(L, 4)) {
        lua_pushvalue(L, 4);
        arg_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LUA_TIMER_T *timer = (LUA_TIMER_T *)lua_newuserdata(L, sizeof(LUA_TIMER_T));
    if (!timer) {
        luaL_unref(L, LUA_REGISTRYINDEX, func_ref);
        if (arg_ref != LUA_NOREF) 
            luaL_unref(L, LUA_REGISTRYINDEX, arg_ref);

        lua_pushnil(L);
        return 1;        
    }
    memset(timer, 0, sizeof(LUA_TIMER_T));
    timer->timeout = timeout;
    timer->type = type;
    timer->func_ref = func_ref;
    timer->arg_ref = arg_ref;
    timer->L = L;
    tal_sw_timer_create(__timer_callback, timer, &timer->os_timer);
    if (OPRT_OK != tal_sw_timer_start(timer->os_timer, timer->timeout, timer->type)) {
        tal_sw_timer_delete(timer->os_timer);
        tal_free(timer);
        lua_pushnil(L);
        return 1;
    }

    // return timer
	luaL_getmetatable(L, TIMER_MT);
	lua_setmetatable(L, -2);
    // lua_pushlightuserdata(L, timer);
    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_timer[] = {
    {"start",                           ROREG_FUNC(lua_timer_new)},
    {"trigger",                         ROREG_FUNC(lua_timer_trigger)},
    {"stop",                            ROREG_FUNC(lua_timer_stop)},
    {"delete",                          ROREG_FUNC(lua_timer_delete)},
    {"ONCE",                            ROREG_INT(TAL_TIMER_ONCE)},
    {"CYCLE",                           ROREG_INT(TAL_TIMER_CYCLE)},
    { NULL,                             ROREG_INT(0) }
};

int luaopen_timer(lua_State *L)
{
    luaL_newmetatable(L, TIMER_MT);

    luaL_newlib2(L, reg_timer);
    return 1;
}
