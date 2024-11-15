
#include "tal_event.h"
#include "lua.h"
#include "lauxlib.h"
#include "tkl_mutex.h"
#include "lua_msgbus.h"
typedef struct {
    lua_State *L;
    int cb_ref;
    EVENT_SUBSCRIBE_CB event_subscrite_cb;
    char desc[EVENT_DESC_MAX_LEN+1];
    char name[EVENT_NAME_MAX_LEN + 1];
} CallbackData;
#define MAX_CALLBACKS 20 // 根据你的需求设置最大回调数量

static TKL_MUTEX_HANDLE s_callbacks_mutex = NULL;


static int lua_event_callback_bridge_core(int index,void *data);

#define TY_EVENT_CB_DEFINE(n)   \
    static int lua_event_callback_bridge_ ## n (void *param) \
    { \
        lua_event_callback_bridge_core(n,param); \
        return 1;\
    }

#define TY_EVENT_CB_DECLARE(n)  static int lua_event_callback_bridge_ ## n (void *param);

TY_EVENT_CB_DECLARE(0)
TY_EVENT_CB_DECLARE(1)
TY_EVENT_CB_DECLARE(2)
TY_EVENT_CB_DECLARE(3)
TY_EVENT_CB_DECLARE(4)
TY_EVENT_CB_DECLARE(5)
TY_EVENT_CB_DECLARE(6)
TY_EVENT_CB_DECLARE(7)
TY_EVENT_CB_DECLARE(8)
TY_EVENT_CB_DECLARE(9)
TY_EVENT_CB_DECLARE(10)
TY_EVENT_CB_DECLARE(11)
TY_EVENT_CB_DECLARE(12)
TY_EVENT_CB_DECLARE(13)
TY_EVENT_CB_DECLARE(14)
TY_EVENT_CB_DECLARE(15)
TY_EVENT_CB_DECLARE(16)
TY_EVENT_CB_DECLARE(17)
TY_EVENT_CB_DECLARE(18)
TY_EVENT_CB_DECLARE(19)


CallbackData s_callbacks[] = {
    {NULL, LUA_NOREF, lua_event_callback_bridge_0,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_1,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_2,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_3,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_4,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_5,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_6,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_7,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_8,{0},{0}},
    {NULL, LUA_NOREF, lua_event_callback_bridge_9,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_10,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_11,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_12,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_13,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_14,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_15,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_16,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_17,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_18,{0},{0}},
    { NULL, LUA_NOREF, lua_event_callback_bridge_19,{0},{0}}
};


TY_EVENT_CB_DEFINE(0)
TY_EVENT_CB_DEFINE(1)
TY_EVENT_CB_DEFINE(2)
TY_EVENT_CB_DEFINE(3)
TY_EVENT_CB_DEFINE(4)
TY_EVENT_CB_DEFINE(5)
TY_EVENT_CB_DEFINE(6)
TY_EVENT_CB_DEFINE(7)
TY_EVENT_CB_DEFINE(8)
TY_EVENT_CB_DEFINE(9)
TY_EVENT_CB_DEFINE(10)
TY_EVENT_CB_DEFINE(11)
TY_EVENT_CB_DEFINE(12)
TY_EVENT_CB_DEFINE(13)
TY_EVENT_CB_DEFINE(14)
TY_EVENT_CB_DEFINE(15)
TY_EVENT_CB_DEFINE(16)
TY_EVENT_CB_DEFINE(17)
TY_EVENT_CB_DEFINE(18)
TY_EVENT_CB_DEFINE(19)


static int l_ty_publish_event(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    void *data = NULL;

    // 检查第二个参数是否为表类型或 nil
    if (!lua_isnil(L, 2) && !lua_istable(L, 2)) {
        lua_pushvalue(L, 2);
        int data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        data = (void *)lua_touserdata(L, data_ref);
        luaL_unref(L, LUA_REGISTRYINDEX, data_ref);
    }

    OPERATE_RET ret = tal_event_publish(name, data);

    // 根据返回值处理结果
    if (ret != OPRT_OK) {
        luaL_error(L, "Failed to publish event: %d", ret);
        return 0;
    }

    lua_pushinteger(L, ret);
    return 1;
}


int lua_event_handler(lua_State *L, void* ptr)
{

    TUYA_RTOS_MSG_T* msg = (TUYA_RTOS_MSG_T*)lua_topointer(L, -1);
    int index = msg->arg1;
    CallbackData *callback_data = &s_callbacks[index];

    if (callback_data->cb_ref == LUA_NOREF) {
        return -1; // 错误处理，如果全局变量未设置
    }
    lua_geti(L, LUA_REGISTRYINDEX, callback_data->cb_ref);
    if (!lua_isnil(L, -1)) {
        lua_pushlightuserdata(L, msg->ptr);
        lua_call(L, 1, 1);
        int result = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        return result; // 将回调函数的返回值作为 lua_event_handler 函数的返回值
    }
    return 1;
}


static int lua_event_callback_bridge_core(int index,void *data)
{

    CallbackData *callback_data = &s_callbacks[index];

    if(callback_data == NULL) {
        return 0;
    }
    // lua_State *L = callback_data->L;
    // int cb_ref = callback_data->cb_ref;

    // if (L == NULL || cb_ref == LUA_NOREF) {
    //     return -1; // 错误处理，如果全局变量未设置
    // }

    // lua_rawgeti(L, LUA_REGISTRYINDEX, cb_ref);
    // lua_pushlightuserdata(L, data);
    // lua_call(L, 1,1);
    // int result = (int)lua_tointeger(L, -1);
    // lua_pop(L, 1);

    TUYA_RTOS_MSG_T msg;
    msg.handler =lua_event_handler;
    msg.arg1=index;
    msg.ptr = data;

    lua_msgbus_put(&msg,0);
    return 1;
}

static int l_ty_subscribe_event(lua_State *L)
{
    int i = 0;
    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    int type = luaL_checkinteger(L, 4);
    int index = MAX_CALLBACKS;
    tkl_mutex_lock(s_callbacks_mutex);
    for (i = 0; i < MAX_CALLBACKS; i++) {
        if (s_callbacks[i].L == NULL) {
            s_callbacks[i].L = L;
            s_callbacks[i].cb_ref = cb_ref;
            strncpy(s_callbacks[i].name,name,EVENT_NAME_MAX_LEN);
            strncpy(s_callbacks[i].desc,desc,EVENT_DESC_MAX_LEN);
            index = i;
            break;
        }
    }
    tkl_mutex_unlock(s_callbacks_mutex);
    if (index == MAX_CALLBACKS) {
        luaL_error(L, "No available callback slots");
        return 0;
    }
    OPERATE_RET ret = tal_event_subscribe(name, desc, s_callbacks[index].event_subscrite_cb, (SUBSCRIBE_TYPE_E)type);

    lua_pushinteger(L, ret);
    return 1;
}

static int l_ty_unsubscribe_event(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);
    int i = 0;
    int found = 0;


    tkl_mutex_lock(s_callbacks_mutex);
    for (i = 0; i < MAX_CALLBACKS; i++) {
        if (strcmp(s_callbacks[i].name,name) == 0 && strcmp(s_callbacks[i].desc,desc)) {
            found = 1;
            break;
        }
    }
    tkl_mutex_unlock(s_callbacks_mutex);

    if (!found || i >=MAX_CALLBACKS ) {
        luaL_error(L, "Callback not found for event '%s'", name);
        return 0;
    }

    OPERATE_RET ret = tal_event_unsubscribe(name,desc,s_callbacks[i].event_subscrite_cb);
    if (ret == OPRT_OK) {
        s_callbacks[i].L = NULL;
        s_callbacks[i].cb_ref = LUA_NOREF;
        s_callbacks[i].name[0] = '\0';
        s_callbacks[i].desc[0] = '\0';
        s_callbacks[i].event_subscrite_cb = NULL;
    }

    lua_pushinteger(L, ret);
    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t ty_event_lib[] = {
    {"tal_event_subscribe", ROREG_FUNC(l_ty_subscribe_event)},
    {"tal_event_publish", ROREG_FUNC(l_ty_publish_event)},
    {"tal_event_unsubscribe", ROREG_FUNC(l_ty_unsubscribe_event)},
// #if defined(ENABLE_CELLULAR) && (ENABLE_CELLULAR==1)
//     {"CELL_REGISTION_EVENT",    ROREG_STR(CELL_REGISTION_EVENT)},
//     {"CELL_PDP_EVENT",          ROREG_STR(CELL_PDP_EVENT)},
//     {"CELL_SIM_EVENT",          ROREG_STR(CELL_SIM_EVENT)},
//     {"CELL_NET_ISSUE_EVENT",    ROREG_STR(CELL_NET_ISSUE_EVENT)},
// #endif
    {"EVENT_RESET",             ROREG_STR(EVENT_RESET)},
    // {"EVENT_INIT",              ROREG_STR(EVENT_INIT)},
    {"EVENT_MQTT_CONNECTED",    ROREG_STR(EVENT_MQTT_CONNECTED)},
    {"EVENT_MQTT_DISCONNECTED", ROREG_STR(EVENT_MQTT_DISCONNECTED)},
    // {"EVENT_OTA_PROCESS_NOTIFY",  ROREG_STR(EVENT_OTA_PROCESS_NOTIFY)},
    // {"EVENT_OTA_FAILED_NOTIFY",  ROREG_STR(EVENT_OTA_FAILED_NOTIFY)},
    // {"EVENT_OTA_FINISHED_NOTIFY",  ROREG_STR(EVENT_OTA_FINISHED_NOTIFY)},
    {"SUBSCRIBE_TYPE_NORMAL",   ROREG_INT(SUBSCRIBE_TYPE_NORMAL)},
    {"SUBSCRIBE_TYPE_EMERGENCY",   ROREG_INT(SUBSCRIBE_TYPE_EMERGENCY)},
    {"SUBSCRIBE_TYPE_ONETIME",   ROREG_INT(SUBSCRIBE_TYPE_ONETIME)},

    {NULL, ROREG_INT(0)}
};

LUAMOD_API int luaopen_event(lua_State *L)
{
    tkl_mutex_create_init(&s_callbacks_mutex);
    luaL_newlib2(L, ty_event_lib);
    return 1;
}


/*
local ty_event = require("ty_event_lib")

local function event_callback(data)
    -- Process the event data here
end

local name = ty_event.cell_pdp_event
local desc = "event_description"
local type = SUBSCRIBE_TYPE_NORMAL

local ret = ty_event.ty_subscribe_event(name, desc, event_callback, type)



local event_name = "event_name"
local event_data = {
    key1 = "value1",
    key2 = "value2"
}

local ret = mylib.ty_publish_event(event_name, event_data)
 */