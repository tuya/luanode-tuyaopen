#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "tal_log.h"
#include "tal_kv.h"

#define LUA_FSKV_MAX_SIZE  4096
static int l_tal_kv_set(lua_State *L)
{
    size_t len;
    luaL_Buffer buff;
    luaL_buffinit(L, &buff);
    const char* key = luaL_checkstring(L, 1);
    //luaL_addchar(&buff, 0xA5);
    int type = lua_type(L, 2);
    switch (type)
    {
    case LUA_TBOOLEAN:
        luaL_addchar(&buff, LUA_TBOOLEAN);
        bool val = lua_toboolean(L, 2);
        luaL_addlstring(&buff, (const char*)&val, sizeof(val));
        break;
    case LUA_TNUMBER:
        if (lua_isinteger(L, 2)) {
            luaL_addchar(&buff, LUA_TNUMBER); // 自定义类型
            lua_Integer val = luaL_checkinteger(L, 2);
            luaL_addlstring(&buff, (const char*)&val, sizeof(val));
        }
        else {
            luaL_addchar(&buff, LUA_TNUMBER);
            lua_getglobal(L, "pack");
            if (lua_isnil(L, -1)) {
                PR_ERR("float number need pack lib");
                lua_pushboolean(L, 0);
                return 1;
            }
            lua_getfield(L, -1, "pack");
            lua_pushstring(L, ">f");
            lua_pushvalue(L, 2);
            lua_call(L, 2, 1);
            if (lua_isstring(L, -1)) {
                const char* val = luaL_checklstring(L, -1, &len);
                luaL_addlstring(&buff, val, len);
            }
            else {
                PR_ERR("kdb store number fail!!");
                lua_pushboolean(L, 0);
                return 1;
            }
        }
        break;
    case LUA_TSTRING:
    {
        luaL_addchar(&buff, LUA_TSTRING);
        const char* val = luaL_checklstring(L, 2, &len);
        luaL_addlstring(&buff, val, len);
        break;
    }
    case LUA_TTABLE:
    {
        lua_settop(L, 2);
        lua_getglobal(L, "json");
        if (lua_isnil(L, -1)) {
            PR_ERR("miss json lib, not support table value");
            lua_pushboolean(L, 0);
            return 1;
        }
        lua_getfield(L, -1, "encode");
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, 2);
            lua_call(L, 1, 1);
            if (lua_isstring(L, -1)) {
                luaL_addchar(&buff, LUA_TTABLE);
                const char* val = luaL_checklstring(L, -1, &len);
                luaL_addlstring(&buff, val, len);
            }
            else {
                PR_ERR("json.encode(val) report error");
                lua_pushboolean(L, 0);
                return 1;
            }
        }
        else {
            PR_ERR("miss json.encode, not support table value");
            lua_pushboolean(L, 0);
            return 1;
        }
        break;
    }
    default:
    {
        PR_ERR("function/userdata/nil/thread isn't allow");
        lua_pushboolean(L, 0);
        return 1;
    }
    }
    if (buff.n > LUA_FSKV_MAX_SIZE) {
        PR_ERR("value too big %d max %d", buff.n, LUA_FSKV_MAX_SIZE);
        lua_pushboolean(L, -1);
        return 1;
    }
    int ret = tal_kv_set(key, (const uint8_t*)buff.b, (const unsigned int)buff.n);
    lua_pushinteger(L, ret == buff.n ? 1 : 0);
    return 1;

    lua_pushinteger(L,ret);
}

static int l_tal_kv_get(lua_State *L)
{
    // luaL_Buffer buff;
    const char* key = luaL_checkstring(L, 1);
    const char* skey = luaL_optstring(L, 2, "");
    // luaL_buffinitsize(L, &buff, 8192);

    uint8_t *rbuff = NULL;
    char *buff = NULL;
    int buff_len = 0;
    int ret = tal_kv_get(key, &rbuff,(unsigned int*)&buff_len);
    if (ret != OPRT_OK) {
        return 0; // 对应的KEY不存在
    }
    if (rbuff == NULL) {
        return 0;
    }
    buff = (char*)rbuff;

    switch(buff[0]) {
    case LUA_TBOOLEAN:
        lua_pushboolean(L, buff[1]);
        break;
    case LUA_TNUMBER:
        lua_getglobal(L, "pack");
        lua_getfield(L, -1, "unpack");
        lua_pushlstring(L, (char*)(buff + 1), ret - 1);
        lua_pushstring(L, ">f");
        lua_call(L, 2, 2);
        // _, val = pack.unpack(data, ">f")
        break;
    case LUA_TSTRING:
        lua_pushlstring(L, (const char*)(buff + 1), ret - 1);
        break;
    case LUA_TTABLE:
        lua_getglobal(L, "json");
        lua_getfield(L, -1, "decode");
        lua_pushlstring(L, (const char*)(buff + 1), ret - 1);
        lua_call(L, 1, 1);
        if (strlen(skey) > 0 && lua_istable(L, -1)) {
            lua_getfield(L, -1, skey);
        }
        break;
    default :
        PR_ERR("bad value prefix %02X", buff[0]);
        lua_pushnil(L);
        break;
    }
    if (rbuff)
        tal_kv_free((uint8_t*)rbuff);
    return 1;
}

static int l_tal_kv_del(lua_State *L)
{
    const char* key = luaL_checkstring(L, 1);
    if (key == NULL) {
        lua_pushinteger(L,-1);
    }
    else {
        int ret = tal_kv_del(key);
        lua_pushinteger(L,ret);
    }
    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_kv[] = {
    {"kv_set",          ROREG_FUNC(l_tal_kv_set)},
    {"kv_get",          ROREG_FUNC(l_tal_kv_get)},
    {"kv_del",          ROREG_FUNC(l_tal_kv_del)},
    { NULL,             ROREG_INT(0) }
};

LUAMOD_API int luaopen_kv(lua_State *L)
{
    luaL_newlib2(L, reg_kv);
    return 1;
}