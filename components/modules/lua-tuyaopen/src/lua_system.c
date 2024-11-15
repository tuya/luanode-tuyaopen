/*
@module  rtos
@summary RTOS底层操作库
@version 1.0
@date    2020.03.30
@tag LUAT_CONF_BSP
*/
#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "tkl_system.h"
#include "tkl_output.h"
#include "tkl_queue.h"
#include "tkl_mutex.h"
#include "lua.h"
#include "lua_timer.h"
#include "lua_msgbus.h"
#include "tkl_memory.h"
#include "lua_timer.h"

#include "lua_timer.h"
#include "tal_log.h"
#include "tuya_iot_config.h"
/*
接受并处理底层消息队列.
@api    rtos.receive(timeout)
@int  超时时长,通常是-1,永久等待
@return msgid          如果是定时器消息,会返回定时器消息id及附加信息, 其他消息由底层决定,不向lua层进行任何保证.
--  本方法通过sys.run()调用, 普通用户不要使用
rtos.receive(-1)
*/
static int l_system_receive_msg(lua_State *L) {
    TUYA_RTOS_MSG_T msg = {0};
    int re = 0;
    re = lua_msgbus_get(&msg, luaL_checkinteger(L, 1));
    if (!re) {
        //LLOGD("rtos_msg got, invoke it handler=%08X", msg.handler);
        lua_pushlightuserdata(L, (void*)(&msg));
        return msg.handler(L, msg.ptr);
    }
    else {
        //LLOGD("rtos_msg get timeout");
        lua_pushinteger(L, -1);
        return 1;
    }
}


/*
设备重启
@api    rtos.reboot()
@return nil          无返回值
-- 立即重启设备
rtos.reboot()
*/
int l_system_reboot(lua_State *L) {
    tkl_system_reset();
    return 0;
}

/**
 * @brief get free heap size
 * 
 */
int l_system_free_heap(lua_State *L) {
    int free_heap = tkl_system_get_free_heap_size();
    lua_pushinteger(L, free_heap);
    return 1;
}

/**
 * @brief 
 * 
 */
 int l_system_info(lua_State *L) {
    lua_pushstring(L, EXAMPLE_NAME);
    lua_pushstring(L, EXAMPLE_VER);
    lua_pushstring(L, PLATFORM_NAME);
    lua_pushstring(L, PLATFORM_CHIP);
    lua_pushstring(L, BUILD_DATE);
    return 5;
 }
//-----------------------------------------------------------------

/*
获取固件编译日期
@api    rtos.buildDate()
@return string 固件编译日期
@usage
-- 获取编译日期
local d = rtos.buildDate()
*/
static int l_system_build_date(lua_State *L) {
    lua_pushstring(L, __DATE__);
    return 1;
}


extern char custom_search_paths[4][128];

/*
设置自定义lua脚本搜索路径,优先级高于内置路径
@api    rtos.setPaths(pathA, pathB, pathC, pathD)
@string 路径A, 例如 "/sdcard/%s.luac",若不传值,将默认为"",另外,最大长度不能超过23字节
@string 路径B, 例如 "/sdcard/%s.lua"
@string 路径C, 例如 "/lfs2/%s.luac"
@string 路径D, 例如 "/lfs2/%s.lua"
@usage
-- 挂载sd卡或者spiflash后
rtos.setPaths("/sdcard/user/%s.luac", "/sdcard/user/%s.lua")
require("sd_user_main") -- 将搜索并加载 /sdcard/user/sd_user_main.luac 和 /sdcard/user/sd_user_main.lua
*/
static int l_system_set_paths(lua_State *L) {
    size_t len = 0;
    const char* str = NULL;
    for (size_t i = 0; i < 4; i++)
    {
        if (lua_isstring(L, i +1)) {
            str = luaL_checklstring(L, i+1, &len);
            memcpy(custom_search_paths[i], str, len + 1);
        }
        else {
            custom_search_paths[i][0] = 0x00;
        }
    }
    return 0;
}

static int l_system_poweron_reason(lua_State *L) {
    int rst=  tkl_system_get_reset_reason(NULL);
    lua_pushinteger(L,rst);
    return 1;
}


static int l_system_sleep_ms(lua_State *L) {

    int ms = luaL_optnumber(L,1,100);
    tkl_system_sleep(ms);
    return 0;
}

static int l_system_get_sys_ms(lua_State *L) {

    SYS_TIME_T sys_ms = tkl_system_get_millisecond();
    lua_pushnumber(L,(lua_Number)sys_ms);
    return 1;
}

static int l_system_logout(lua_State *L)
{
    int nargs = lua_gettop(L); // 获取参数数量
    luaL_Buffer b;
    struct tm *timeinfo;
    char time_str[20];
    timeinfo = rtos_localtime(NULL);
    strftime(time_str, sizeof(time_str), "[%m-%d %H:%M:%S", timeinfo);


    luaL_where(L, 1); // 获取调用者的信息
    const char *caller_info = lua_tostring(L, -1);
    const char *filename = strrchr(caller_info, '/');
    if (filename) {
        filename++; // 移动到分隔符之后的字符
    } else {
        filename = caller_info;
    }
    luaL_buffinit(L, &b);

    luaL_addstring(&b, time_str); // 将时间字符串添加到缓冲区
    luaL_addstring(&b, ":lua ]["); // 添加分隔符
    luaL_addstring(&b, filename); // 将调用者信息添加到缓冲区
    luaL_addstring(&b,  "]");
    lua_pop(L, 1); // 从栈中删除调用者信息


    for (int i = 1; i <= nargs; i++) {
        const char *msg = luaL_tolstring(L, i, NULL); // 将参数转换为字符串
        luaL_addstring(&b, msg); // 将字符串添加到缓冲区
        lua_pop(L, 1); // 从栈中删除转换后的字符串
    }

    luaL_pushresult(&b); // 将缓冲区的内容推送到栈中
    const char *final_msg = lua_tostring(L, -1); // 获取最终的字符串
    tkl_log_output("%s\n", final_msg); // 调用底层日志输出函数
    lua_pop(L, 1); // 从栈中删除最终的字符串

    return 0; // 返回值的数量
}
/* #include "tkl_semaphore.h"
static int l_createSemaphore(lua_State *L) {


    TKL_SEM_HANDLE semaphore = NULL;
    int sem_cnt = luaL_optnumber(L,1,1);
    int sem_max = luaL_optnumber(L,1,2);
    tkl_semaphore_create_init(&semaphore,sem_cnt,sem_max);
    if (semaphore == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to create semaphore");
        return 2;
    }
    lua_pushlightuserdata(L, semaphore);
    return 1;
}

static int l_takeSemaphore(lua_State *L) {
    TKL_SEM_HANDLE semaphore = (TKL_SEM_HANDLE)lua_touserdata(L, 1);
    int result = tkl_semaphore_post(semaphore);
    lua_pushboolean(L, result == OPRT_OK);
    return 1;
}

static int l_semaphore_wait(lua_State *L) {
    TKL_SEM_HANDLE semaphore = (TKL_SEM_HANDLE)lua_touserdata(L, 1);
    int timeout = luaL_optnumber(L,2,10);
    int result = tkl_semaphore_wait(semaphore,timeout);
    lua_pushboolean(L, result == OPRT_OK);
    return 1;
}
 */

static int l_system_get_random(lua_State *L)
{
    int range = luaL_optnumber(L,1,0xff);
    int random =  tkl_system_get_random(range);
    lua_pushinteger(L,random);
    return 1;
}

#include "tal_sleep.h"
static int l_system_lpg(lua_State *L)
{
    int enable = luaL_checkinteger(L,1);
    if (enable) {
        tal_cpu_set_lp_mode(TRUE);
    }
    else {
        tal_cpu_set_lp_mode(FALSE);
    }
    return 0;
}
#include "tal_time_service.h"

// 返回时间顺序：秒(0-59)，分(0-59)，时(0-23)，日(1-31)，月(0-11)，年(1900~)，星期(0-6 0-Sunday...6-Saturday)
static int l_system_time_get(lua_State *L)
{
    POSIX_TM_S              tm = {0};
    int                   *p_tm = NULL;

    p_tm = (int*)&tm;
    if (OPRT_OK != tal_time_get(&tm)) {
        return OPRT_COM_ERROR;
    }

    lua_newtable(L);                                // 创建一个表
    for (unsigned short i = 0; i < 7; i++) {
		lua_pushinteger(L, i + 1);
		lua_pushinteger(L, p_tm[i]);                //将数组的数据入栈
		lua_settable(L, -3);                        //写入table
    }
    return 1;
}


//------------------------------------------------------------------
#include "rotable2.h"
static const rotable_Reg_t reg_system[] =
{
    // { "timer_start" ,      ROREG_FUNC(l_system_timer_start)},
    // { "timer_stop",        ROREG_FUNC(l_system_timer_stop)},
    // { "timer_trigger",        ROREG_FUNC(l_system_timer_trigger)},
    // { "timer_delete",        ROREG_FUNC(l_system_timer_delete)},
    // { "msg_recv",       ROREG_FUNC(l_system_receive_msg)},
    { "reset",            ROREG_FUNC(l_system_reboot)},
    { "reset_reason",    ROREG_FUNC(l_system_poweron_reason)},
    // { "buildDate",         ROREG_FUNC(l_system_build_date)},
    // { "setPaths",          ROREG_FUNC(l_system_set_paths)},
    { "sleep_ms",          ROREG_FUNC(l_system_sleep_ms)},
    { "delay_ms",          ROREG_FUNC(l_system_sleep_ms)},
    // { "get_sys_ms",         ROREG_FUNC(l_system_get_sys_ms)},
    { "log",                ROREG_FUNC(l_system_logout)},
    { "random",         ROREG_FUNC(l_system_get_random)},
    // { "lp_mode",          ROREG_FUNC(l_system_lpg)},
    { "now",           ROREG_FUNC(l_system_time_get)},
    { "heap",         ROREG_FUNC(l_system_free_heap)},
    { "info",         ROREG_FUNC(l_system_info)},
    { "INF_TIMEOUT",       ROREG_INT(-1)},
    { "MSG_TIMER",         ROREG_INT(MSG_TIMER)},
	{ NULL,                ROREG_INT(0) }
};
LUAMOD_API int luaopen_system (lua_State *L) {
    luaL_newlib2(L, reg_system);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "tuyaos");
    return 1;
}
