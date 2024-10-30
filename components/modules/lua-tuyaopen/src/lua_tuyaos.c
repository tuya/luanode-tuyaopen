#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lua_msgbus.h"
#include "dp_schema.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_iot.h"
#include "tuya_ota.h"
#include "tuya_config.h"
#include "tal_api.h"
#include "tkl_output.h"
#include "tkl_memory.h"
#include "netmgr.h"
#include "tuya_iot_dp.h"
#include "tuya_iot_config.h"

#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#include "netconn_wifi.h"
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
#include "netconn_wired.h"
#endif
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
#include "lwip_init.h"
#endif

static int s_gw_status_cb_registry = LUA_NOREF;
static int s_dev_obj_dp_cb_registry = LUA_NOREF;
static int s_gw_ug_cb_registry = LUA_NOREF;
static int s_dev_raw_dp_cb_registry = LUA_NOREF;
static int s_active_shorturl_cb_registry = LUA_NOREF;
static int s_fota_inform_cb_registry = LUA_NOREF;
static int s_fota_process_cb_registry = LUA_NOREF;
static int s_fota_end_cb_registry = LUA_NOREF;

/* Tuya device handle */
tuya_iot_client_t client;

static int l_gw_status_changed_cb(lua_State *L, void* ptr)
{
    TUYA_RTOS_MSG_T* msg = (TUYA_RTOS_MSG_T*)lua_topointer(L, -1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_gw_status_cb_registry);
    if (!lua_isnil(L, -1)) {
        lua_pushinteger(L, msg->arg1);
        lua_call(L, 1, 0);
    }
    return 0;
}

static void __recv_status_changed_cb(netmgr_status_e status)
{
    TUYA_RTOS_MSG_T msg;
    msg.arg1 = status;
    msg.handler = l_gw_status_changed_cb;
    lua_msgbus_put(&msg,0);
}

static void push_ty_obj_dp_s_array_to_lua(lua_State *L, dp_obj_t *dps, unsigned int dps_cnt)
{
    lua_newtable(L);

    for (unsigned int i = 0; i < dps_cnt; ++i) {
        dp_obj_t *dp = &dps[i];

        lua_pushinteger(L, i + 1); // Lua数组索引从1开始
        lua_newtable(L); // 创建一个新表来存储TY_OBJ_DP_S结构体的成员

        lua_pushstring(L, "dpid");
        lua_pushinteger(L, dp->id);
        lua_settable(L, -3);

        lua_pushstring(L, "type");
        lua_pushinteger(L, dp->type);
        lua_settable(L, -3);

        // 处理TY_OBJ_DP_VALUE_U联合体，需要根据dp->type来判断具体类型
        // ...
        lua_pushstring(L,"value");
        switch(dp->type)
        {
            case  PROP_BOOL:
                lua_pushboolean(L,dp->value.dp_bool);
            break;
            case PROP_VALUE:
                lua_pushinteger(L,dp->value.dp_value);
            break;
            case PROP_STR:
                lua_pushfstring(L,dp->value.dp_str);
            break;
            case PROP_ENUM:
                lua_pushinteger(L,dp->value.dp_enum);
            break;
        }
        lua_settable(L, -3);

        lua_pushstring(L, "time_stamp");
        lua_pushinteger(L, dp->time_stamp);
        lua_settable(L, -3);

        lua_settable(L, -3); // 将新表添加到数组中
    }
}

static int push_ty_recv_obj_dp_s_to_lua(lua_State *L, dp_obj_recv_t *obj)
{
    lua_newtable(L);

    lua_pushstring(L, "cmd_tp");
    lua_pushinteger(L, obj->cmd_tp);
    lua_settable(L, -3);

    lua_pushstring(L, "dps_cnt");
    lua_pushinteger(L, obj->dpscnt);
    lua_settable(L, -3);

    lua_pushstring(L, "dps");

    push_ty_obj_dp_s_array_to_lua(L, obj->dps, obj->dpscnt);

    lua_settable(L, -3);

    return 1; // 返回值表示我们将一个值压入了Lua栈
}

static int l_dev_obj_dp_cmd_cb(lua_State *L, void *ptr)
{

    dp_obj_recv_t *dp = (dp_obj_recv_t*)ptr;

    lua_rawgeti(L, LUA_REGISTRYINDEX, s_dev_obj_dp_cb_registry);

    push_ty_recv_obj_dp_s_to_lua(L,(dp_obj_recv_t*)dp);
    tkl_system_free(dp);
    lua_call(L, 1, 0);

    return 0;
}


static void __recv_obj_dp_cb(const dp_obj_recv_t *dp)
{
    TUYA_RTOS_MSG_T msg = {0};
    dp_obj_recv_t *obj_dps = (dp_obj_recv_t *)tkl_system_malloc(sizeof(dp_obj_recv_t) + (dp->dpscnt * sizeof(dp_obj_t)));
    if (NULL == obj_dps) {
        PR_ERR("malloc err:%d", dp->dpscnt);
        return;
    }

    memset(obj_dps, 0, (sizeof(dp_obj_recv_t) + (dp->dpscnt * sizeof(dp_obj_t))));
    obj_dps->cmd_tp = dp->cmd_tp;
    obj_dps->dtt_tp = dp->dtt_tp;
    obj_dps->devid = dp->devid;
    obj_dps->dpscnt = dp->dpscnt;

    for (int i = 0; i < dp->dpscnt; i ++) {
        memcpy(&obj_dps->dps[i] , &dp->dps[i],sizeof(dp_obj_t));
    }

    msg.ptr = obj_dps;
    msg.handler = l_dev_obj_dp_cmd_cb;
    lua_msgbus_put(&msg,0);
}

static int push_ty_recv_raw_dp_s_to_lua(lua_State *L, dp_raw_recv_t *obj)
{
    lua_newtable(L);

    lua_pushstring(L, "cmd_tp");
    lua_pushinteger(L, obj->cmd_tp);
    lua_settable(L, -3);

    lua_pushstring(L, "dpid");
    lua_pushinteger(L, obj->dp.id);
    lua_settable(L, -3);

    lua_pushstring(L, "dp_len");
    lua_pushinteger(L, obj->dp.len);
    lua_settable(L, -3);

    lua_pushstring(L, "dp_data");
    lua_pushlstring(L, (const char *)obj->dp.data, obj->dp.len);
    lua_settable(L, -3);

    return 1; // 返回值表示我们将一个值压入了Lua栈
}

static int l_dev_raw_dp_cb(lua_State *L,void *ptr)
{
    dp_raw_recv_t *dp = (dp_raw_recv_t*)ptr;

    if (dp == NULL) {
        return 0;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_dev_raw_dp_cb_registry);

    push_ty_recv_raw_dp_s_to_lua(L,(dp_raw_recv_t*)dp);
    tkl_system_free(dp);
    lua_call(L, 1, 0);

    return 0;
}

static void __recv_raw_dp_cb(const dp_raw_recv_t *dp)
{
    TUYA_RTOS_MSG_T msg = {0};
    dp_raw_recv_t *raw_dp = tkl_system_malloc(sizeof(dp_raw_recv_t)+dp->dp.len);

    raw_dp->cmd_tp = dp->cmd_tp;
    raw_dp->dtt_tp = dp->dtt_tp;
    raw_dp->dp.id = dp->dp.id;
    raw_dp->dp.len = dp->dp.len;

    memcpy(raw_dp->dp.data,dp->dp.data,dp->dp.len);

    msg.handler = l_dev_raw_dp_cb;
    msg.ptr = raw_dp;
    lua_msgbus_put(&msg,0);
}

static int l_dev_get_qr_cb(lua_State *L,void *ptr)
{
    char *shorturl = ptr;
    if (s_active_shorturl_cb_registry == LUA_NOREF && ptr) {
        return 0;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_active_shorturl_cb_registry);

    lua_pushfstring(L,shorturl);
    tkl_system_free(ptr);
    lua_call(L, 1, 0);

    return 0;
}
/**
 * @brief recv qrcode string, this qrcode was used for activate the device by tuya smartlife APP
 *        you can print it on the screen according libqrcode or translate it to qrcode on https://cli.im/
 * 
 * @param shorturl : the qrcode string
 */
static void __recv_qr_cb(const char *shorturl) {

    TUYA_RTOS_MSG_T msg;
    char *url = NULL;

    msg.handler = l_dev_get_qr_cb;
    if (shorturl) {
        url = tkl_system_malloc(strlen(shorturl)+1);
        if (url == NULL) {
            return ;
        }
        strcpy(url,(char*)shorturl);
        msg.ptr = url;
    }
    lua_msgbus_put(&msg,0);
}

// static int l_gw_ug_cb(lua_State *L,void *ptr)
// {
//     const tuya_ota_msg_t *fw = ptr;
//     lua_rawgeti(L, LUA_REGISTRYINDEX, s_gw_ug_cb_registry);
//     lua_newtable(L);

//     lua_pushstring(L, "fw_tp");
//     lua_pushinteger(L, fw->channel);
//     lua_settable(L, -3);

//     lua_pushstring(L, "fw_sw_ver");
//     lua_pushstring(L,fw->sw_ver);
//     lua_settable(L, -3);

//     lua_pushstring(L, "fw_hmac");
//     lua_pushstring(L,fw->fw_hmac);
//     lua_settable(L, -3);

//     lua_pushstring(L, "fw_md5");
//     lua_pushstring(L,fw->fw_md5);
//     lua_settable(L, -3);

//     lua_pushstring(L, "fw_file_size");
//     lua_pushinteger(L,fw->file_size);
//     lua_settable(L, -3);

//     lua_call(L, 1, 0);

//     return 1;
// }

// /**
//  * @brief recv upgrade message
//  * 
//  * @param fw : the new firmware info
//  * @return int : OPRT_OK-success, others-failed
//  */
// static int __recv_ug_cb(const tuya_ota_msg_t *fw)
// {
//     TUYA_RTOS_MSG_T msg;
//     msg.ptr = fw;
//     msg.handler =l_gw_ug_cb;
//     lua_msgbus_put(&msg,0);
// }

static bool_t safe_lua_toboolean(lua_State *L, int index) {
    if (lua_isboolean(L, index)) {
        return lua_toboolean(L, index);
    }
    return false;
}

/**
 * @brief user defined upgrade notify callback, it will notify device a OTA
 * request received
 *
 * @param client device info
 * @param upgrade the upgrade request info
 * @return void
 */
void __user_upgrade_notify_on(tuya_iot_client_t *client, cJSON *upgrade)
{
    PR_INFO("----- Upgrade information -----");
    PR_INFO("OTA Channel: %d", cJSON_GetObjectItem(upgrade, "type")->valueint);
    PR_INFO("Version: %s", cJSON_GetObjectItem(upgrade, "version")->valuestring);
    PR_INFO("Size: %s", cJSON_GetObjectItem(upgrade, "size")->valuestring);
    PR_INFO("MD5: %s", cJSON_GetObjectItem(upgrade, "md5")->valuestring);
    PR_INFO("HMAC: %s", cJSON_GetObjectItem(upgrade, "hmac")->valuestring);
    PR_INFO("URL: %s", cJSON_GetObjectItem(upgrade, "url")->valuestring);
    PR_INFO("HTTPS URL: %s", cJSON_GetObjectItem(upgrade, "httpsUrl")->valuestring);

    // tuya_ota_msg_t ota = {0};
    // ota.channel = cJSON_GetObjectItem(upgrade, "type")->valueint;
    // strcpy(ota.sw_ver, cJSON_GetObjectItem(upgrade, "version")->valuestring);
    // strcpy(ota.file_size, cJSON_GetObjectItem(upgrade, "size")->valuestring);
    // strcpy(ota.fw_md5, cJSON_GetObjectItem(upgrade, "md5")->valuestring);
    // strcpy(ota.fw_hmac, cJSON_GetObjectItem(upgrade, "hmac")->valuestring);
    // strcpy(ota.fw_url, cJSON_GetObjectItem(upgrade, "httpsUrl")->valuestring);
    // __recv_ug_cb(&ota);
}

/**
 * @brief user defined event handler
 *
 * @param client device info
 * @param event the event info
 * @return void
 */
void __user_event_handler_on(tuya_iot_client_t *client, tuya_event_msg_t *event)
{
    PR_DEBUG("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
    PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());
    switch (event->id) {
    case TUYA_EVENT_BIND_START:
        PR_INFO("Device Bind Start!");
        break;

    /* Print the QRCode for Tuya APP bind */
    case TUYA_EVENT_DIRECT_MQTT_CONNECTED: {
        char buffer[255];
        sprintf(buffer, "https://smartapp.tuya.com/s/p?p=%s&uuid=%s&v=2.0", TUYA_PRODUCT_KEY, TUYA_OPENSDK_UUID);
        __recv_qr_cb(buffer);
    } break;

    /* MQTT with tuya cloud is connected, device online */
    case TUYA_EVENT_MQTT_CONNECTED:
        PR_INFO("Device MQTT Connected!");
        break;

    /* RECV upgrade request */
    case TUYA_EVENT_UPGRADE_NOTIFY:
        __user_upgrade_notify_on(client, event->value.asJSON);
        break;

    /* Sync time with tuya Cloud */
    case TUYA_EVENT_TIMESTAMP_SYNC:
        PR_INFO("Sync timestamp:%d", event->value.asInteger);
        tal_time_set_posix(event->value.asInteger, 1);
        break;
    case TUYA_EVENT_RESET:
        PR_INFO("Device Reset:%d", event->value.asInteger);
        break;

    /* RECV OBJ DP */
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
            PR_DEBUG("devid.%s", dpobj->devid);
        }

        uint32_t index = 0;
        for (index = 0; index < dpobj->dpscnt; index++) {
            dp_obj_t *dp = dpobj->dps + index;
            PR_DEBUG("idx:%d dpid:%d type:%d ts:%u", index, dp->id, dp->type, dp->time_stamp);
            switch (dp->type) {
            case PROP_BOOL: {
                PR_DEBUG("bool value:%d", dp->value.dp_bool);
                break;
            }
            case PROP_VALUE: {
                PR_DEBUG("int value:%d", dp->value.dp_value);
                break;
            }
            case PROP_STR: {
                PR_DEBUG("str value:%s", dp->value.dp_str);
                break;
            }
            case PROP_ENUM: {
                PR_DEBUG("enum value:%u", dp->value.dp_enum);
                break;
            }
            case PROP_BITMAP: {
                PR_DEBUG("bits value:0x%X", dp->value.dp_bitmap);
                break;
            }
            default: {
                PR_ERR("idx:%d dpid:%d type:%d ts:%u is invalid", index, dp->id, dp->type, dp->time_stamp);
                break;
            }
            } // end of switch
        }

        __recv_obj_dp_cb(dpobj);
        // tuya_iot_dp_obj_report(client, dpobj->devid, dpobj->dps, dpobj->dpscnt, 0);

    } break;

    /* RECV RAW DP */
    case TUYA_EVENT_DP_RECEIVE_RAW: {
        dp_raw_recv_t *dpraw = event->value.dpraw;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d", dpraw->cmd_tp, dpraw->dtt_tp);
        if (dpraw->devid != NULL) {
            PR_DEBUG("devid.%s", dpraw->devid);
        }

        uint32_t index = 0;
        dp_raw_t *dp = &dpraw->dp;
        PR_DEBUG("dpid:%d type:RAW len:%d data:", dp->id, dp->len);
        for (index = 0; index < dp->len; index++) {
            PR_DEBUG_RAW("%02x", dp->data[index]);
        }

        __recv_raw_dp_cb(dpraw);
        // tuya_iot_dp_raw_report(client, dpraw->devid, &dpraw->dp, 3);

    } break;

        /* TBD.. add other event if necessary */

    default:
        break;
    }
}

/**
 * @brief user defined network check callback, it will check the network every
 * 1sec, in this demo it alwasy return ture due to it's a wired demo
 *
 * @return true
 * @return false
 */
bool __user_network_check(void)
{
    netmgr_status_e status = NETMGR_LINK_DOWN;
    netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_STATUS, &status);

    __recv_status_changed_cb(status);
}

static int l_tuyaos_start_iot_device(lua_State *L)
{
    const char* pid = luaL_checkstring(L, 1);
    const char* appv = luaL_checkstring(L, 2);
    const char* fw_key = luaL_checkstring(L, 3);

    int ret = OPRT_OK;

    //! open iot development kit runtim init
    cJSON_InitHooks(&(cJSON_Hooks){.malloc_fn = tal_malloc, .free_fn = tal_free});
    // tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);
    // tal_kv_init(&(tal_kv_cfg_t){
    //     .seed = "vmlkasdh93dlvlcy",
    //     .key = "dflfuap134ddlduq",
    // });
    tal_sw_timer_init();
    tal_workq_init();
    // tal_cli_init();
    // tuya_app_cli_init();
    tuya_iot_license_t license;
#if OPERTING_SYSTEM == SYSTEM_LINUX
    if (OPRT_OK != tuya_iot_license_read(&license)) {
        license.uuid = TUYA_OPENSDK_UUID;
        license.authkey = TUYA_OPENSDK_AUTHKEY;
        PR_WARN("Replace the TUYA_OPENSDK_UUID and TUYA_OPENSDK_AUTHKEY contents, otherwise the demo cannot work.\n \
                Visit https://platform.tuya.com/purchase/index?type=6 to get the open-sdk uuid and authkey.");
        return OPRT_AUTHENTICATION_FAIL;
    }
#else
    license.uuid = TUYA_OPENSDK_UUID;
    license.authkey = TUYA_OPENSDK_AUTHKEY;
#endif
    PR_DEBUG("uuid %s, authkey %s", license.uuid, license.authkey);
    /* Initialize Tuya device configuration */
    ret = tuya_iot_init(&client, &(const tuya_iot_config_t){
                                     .software_ver = EXAMPLE_VER,
                                     .productkey = TUYA_PRODUCT_KEY,
                                     .uuid = license.uuid,
                                     .authkey = license.authkey,
                                     .event_handler = __user_event_handler_on,
                                     .network_check = __user_network_check,
                                 });


    // 初始化LWIP
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
    TUYA_LwIP_Init();
#endif

    // network init
    netmgr_type_e type = 0;
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    type |= NETCONN_WIFI;
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
    type |= NETCONN_WIRED;
#endif
    netmgr_init(type);

#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    netmgr_conn_set(NETCONN_WIFI, NETCONN_CMD_NETCFG, &(netcfg_args_t){.type = NETCFG_TUYA_BLE | NETCFG_TUYA_WIFI_AP});
#endif

    PR_DEBUG("tuya_iot_init success");
    /* Start tuya iot task */
    tuya_iot_start(&client);

    for (;;) {
        /* Loop to receive packets, and handles client keepalive */
        tuya_iot_yield(&client);
    }
}

static int l_register_status_change_callback(lua_State *L)
{

    luaL_checktype(L, 1, LUA_TFUNCTION);

    // 如果已经有一个注册的回调，取消引用它
    if (s_gw_status_cb_registry != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, s_gw_status_cb_registry);
    }

    // 引用新的回调函数并将其存储在全局变量中
    lua_pushvalue(L, 1);
    s_gw_status_cb_registry = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}


static int l_register_recv_obj_dp_callback(lua_State *L)
{

    luaL_checktype(L, 1, LUA_TFUNCTION);

    // 如果已经有一个注册的回调，取消引用它
    if (s_dev_obj_dp_cb_registry != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, s_dev_obj_dp_cb_registry);
    }

    // 引用新的回调函数并将其存储在全局变量中
    lua_pushvalue(L, 1);
    s_dev_obj_dp_cb_registry = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

static int l_register_recv_raw_dp_callback(lua_State *L)
{

    luaL_checktype(L, 1, LUA_TFUNCTION);

    // 如果已经有一个注册的回调，取消引用它
    if (s_dev_raw_dp_cb_registry != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, s_dev_raw_dp_cb_registry);
    }

    // 引用新的回调函数并将其存储在全局变量中
    lua_pushvalue(L, 1);
    s_dev_raw_dp_cb_registry = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

static int l_register_shorturl_callback(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);

    // 如果已经有一个注册的回调，取消引用它
    if (s_active_shorturl_cb_registry != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, s_active_shorturl_cb_registry);
    }

    // 引用新的回调函数并将其存储在全局变量中
    lua_pushvalue(L, 1);
    s_active_shorturl_cb_registry = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

static char *my_strdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = (char *)tkl_system_malloc(len);
    if (dst) {
        strcpy(dst, src);
    }
    return dst;
}

dp_obj_t *lua_to_dp_data(lua_State *L, int index)
{
    const char *type_name = lua_typename(L, lua_type(L, index));
    PR_DEBUG("Value at index %d is of type: %s\n", index, type_name);
    int stack_size_before = lua_gettop(L);
    luaL_checktype(L, index, LUA_TTABLE);
    int stack_size_after = lua_gettop(L);
    PR_DEBUG("Stack size before: %d, after: %d\n", stack_size_before, stack_size_after);

    PR_DEBUG("Index before lua_rawlen: %d\n", index);
    PR_DEBUG("Type at index before lua_rawlen: %s\n", lua_typename(L, lua_type(L, index)));
    size_t cnt = lua_rawlen(L, 1);
    PR_DEBUG("Count after lua_rawlen: %zu\n", cnt);
    dp_obj_t *dp_data = (dp_obj_t *)tkl_system_malloc(cnt * sizeof(dp_obj_t));

    for (int i = 0; i <cnt; i++) {
        lua_rawgeti(L, index, i+1);

        lua_getfield(L, -1, "dpid");
        dp_data[i].id = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "type");
        dp_data[i].type = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "value");
        // 根据类型将值存储到联合体中
        switch (dp_data[i].type) {
            case PROP_BOOL:
                dp_data[i].value.dp_bool = lua_toboolean(L, -1);
                break;
            case PROP_VALUE:
                dp_data[i].value.dp_value = luaL_checkinteger(L, -1);
                break;
            case PROP_STR:
                dp_data[i].value.dp_str = my_strdup(luaL_checkstring(L, -1));
                break;
            case PROP_ENUM:
                dp_data[i].value.dp_enum = luaL_checkinteger(L, -1);
                break;
            case PROP_BITMAP:
                dp_data[i].value.dp_bitmap = luaL_checkinteger(L, -1);
                break;
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "time_stamp");
        dp_data[i].time_stamp = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_pop(L, 1); // 弹出包含dp的表
    }

    return dp_data;
}

void free_dp_data(dp_obj_t *dp_data, size_t cnt) {
    // 在这里释放分配的内存（如果有）
    for (size_t i = 0; i < cnt; i++) {
        if (dp_data[i].type == PROP_STR) {
            tkl_system_free(dp_data[i].value.dp_str);
        }
    }
    tkl_system_free(dp_data);
}

int l_dev_report_dp_json_async(lua_State *L)
{
    dp_obj_t *dp_data = lua_to_dp_data(L, 1);
    if  (dp_data == NULL) {
        lua_pushinteger(L, -1);
        return 1;
    }

    unsigned int cnt = luaL_checkinteger(L, 2);
    int ret = tuya_iot_dp_obj_report(tuya_iot_client_get(), tuya_iot_devid_get(tuya_iot_client_get()), dp_data, cnt, 0);

    lua_pushinteger(L, ret);
    free_dp_data(dp_data, cnt);
    return 1;
}

int l_dev_report_dp_raw_sync(lua_State *L)
{

    int dp_id = luaL_checkinteger(L,1);

    luaL_checktype(L, 2, LUA_TTABLE);
    size_t cnt = lua_rawlen(L, 2);
    dp_raw_t *dp = (dp_raw_t *)tkl_system_malloc(sizeof(dp_raw_t)+sizeof(unsigned char)*cnt);
    dp->id = dp_id;
    dp->len = cnt;
    for (size_t i = 0; i < cnt; i++) {
        lua_rawgeti(L, 2, i + 1); // Get the value at index i + 1 (Lua uses 1-based indexing)
        dp->data[i] = (unsigned char)lua_tointeger(L, -1); // Convert the value to an integer and assign it to dp_raw_value
        lua_pop(L, 1); // Remove the value from the stack
    }
    int ret = tuya_iot_dp_raw_report(tuya_iot_client_get(), tuya_iot_devid_get(tuya_iot_client_get()), dp, 0);
    lua_pushinteger(L, ret);
    tkl_system_free(dp);

    return 1;
}

static char s_app_name[64] = {0};
static char s_app_ver[11] = {0};
static int l_tuya_iot_get_app_info(lua_State *L)
{
    lua_pushstring(L,s_app_name);
    lua_pushstring(L,s_app_ver);
    return 2;
}

#include "rotable2.h"
static const rotable_Reg_t reg_tuyaos[] = {
    {"start_device",                    ROREG_FUNC(l_tuyaos_start_iot_device)},
    {"reg_status_change_callback",      ROREG_FUNC(l_register_status_change_callback)},
    {"reg_shorturl_callback",           ROREG_FUNC(l_register_shorturl_callback)},
    {"reg_obj_dp_callback",             ROREG_FUNC(l_register_recv_obj_dp_callback)},
    {"reg_raw_dp_callback",             ROREG_FUNC(l_register_recv_raw_dp_callback)},
    {"report_dp_json_async",            ROREG_FUNC(l_dev_report_dp_json_async)},
    {"report_dp_raw_sync",              ROREG_FUNC(l_dev_report_dp_raw_sync)},
    {"tuya_iot_get_app_info",           ROREG_FUNC(l_tuya_iot_get_app_info)},
    {"PROP_BOOL",                       ROREG_INT(PROP_BOOL)},
    {"PROP_VALUE",                      ROREG_INT(PROP_VALUE)},
    {"PROP_STR",                        ROREG_INT(PROP_STR)},
    {"PROP_ENUM",                       ROREG_INT(PROP_ENUM)},
    {"PROP_BITMAP",                     ROREG_INT(PROP_BITMAP)},
    { NULL,                             ROREG_INT(0) }
};

LUAMOD_API int luaopen_tuyaos(lua_State *L)
{
    luaL_newlib2(L, reg_tuyaos);
    return 1;
}

int lua_set_app_info(char *name,char *ver)
{
    if (name == NULL || ver == NULL ) {
        return OPRT_INVALID_PARM;
    }
    strncpy(s_app_name,name,sizeof(s_app_name));
    strncpy(s_app_ver,ver,sizeof(s_app_ver));

    return 0;

}

/*
function dp_obj_lua_function(obj)

    print("cmd_tp:", obj.cmd_tp)
    print("dps_cnt:", obj.dps_cnt)

    print("dps:")
    for i, dp in ipairs(obj.dps) do
        print("dpid:", dp.dpid)
        print("type:", dp.type)
        -- 访问dp.value，需要根据dp.type来判断具体类型
        -- ...
        print("time_stamp:", dp.time_stamp)
    end
end



function dp_raw_lua_callback(raw_dp)
    -- ... 其他成员访问

    local hex_data = {}
    for i = 1, #raw_dp.data do
        hex_data[i] = string.format("%02X", string.byte(raw_dp.data, i))
    end
    local hex_string = table.concat(hex_data)
    print("data:", hex_string)
end

 */


