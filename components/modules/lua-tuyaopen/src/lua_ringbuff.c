#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "tuya_ringbuf.h"
#include "tkl_memory.h"


static int l_tuya_ring_buff_read(lua_State *L)
{
    unsigned char *p_buff = NULL;
    unsigned char tmp_data[128] = {0};
    uint16_t result;
    unsigned char *p_ringdata;
    TUYA_RINGBUFF_T ringbuff = lua_touserdata(L, 1);
    int len = lua_tointeger(L, 2);
    if (len > 128) {
        p_buff = tkl_system_malloc(len);
        p_ringdata = p_buff;

    }
    else {
        p_ringdata = tmp_data;
    }
    result = tuya_ring_buff_read(ringbuff, (void*)p_ringdata, len);
    if (result > 0) {
        lua_newtable(L);
        for (int i = 0; i < result; i++) {
            lua_pushinteger(L, i + 1);  // Lua的数组索引从1开始
            lua_pushinteger(L, p_ringdata[i]);
            lua_settable(L, -3);
        }
    }
    else {
        lua_pushnil(L);
    }
    if (p_buff){
        tkl_system_free(p_buff);
    }
    return 1;
}

static int l_tuya_ring_buff_get_size(lua_State *L)
{
    TUYA_RINGBUFF_T ringbuff = lua_touserdata(L, 1);
    int size = tuya_ring_buff_used_size_get(ringbuff);
    lua_pushinteger(L,size);
    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_ringbuff[] = {
    {"read",                   ROREG_FUNC(l_tuya_ring_buff_read)},
    {"size",                   ROREG_FUNC(l_tuya_ring_buff_get_size)},
    {NULL, ROREG_INT(0)}
};

LUAMOD_API int luaopen_ringbuff(lua_State *L)
{
    luaL_newlib2(L, reg_ringbuff);
    return 1;
}