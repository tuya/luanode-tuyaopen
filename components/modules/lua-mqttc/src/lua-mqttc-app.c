
/**
 * @file tuya_svc_mqtt_direct.h
 * @brief TUYA mqtt direct service
 * @version 0.1
 * @date 2021-04-06
 *
 * @copyright Copyright 2021 Tuya Inc. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tuya_error_code.h"
#include "mqtt_client_interface.h"
#include "tuya_config_defaults.h"
#include "core_mqtt_config.h"
// #include "core_mqtt.h"
#include "tuya_transporter.h"
#include "backoff_algorithm.h"
#include "tal_api.h"
#include "tkl_output.h"
#include "lua-mqttc.h"
#include "mix_method.h"

#define MQTTC_MT "qmttc"
#define CONVERT_HANDLE_TO_CLIENT(ptr, type, member)   ({(type *)( (char *)ptr - OFFSOF(type,member) );}) 

typedef struct {
    int rt;
    SEM_HANDLE sem_handle;
}custom_mqtt_sync_t;

/**
 * @brief publish data to topic async
 *
 * @param[in] topic : dest topic
 * @param[in] data : publish data
 * @param[in] len : publish data len
 * @param[in] qos : publish qos
 * @param[in] timeout : timeout in seconds, works when qos > 0
 *
 * @note: if qos > 0, retransmit should be done in cb_pub_inform
 *
 * @return int publish message id
 */
OPERATE_RET __custom_mqtt_publish_async(MQTT_CLINT_LUA_T *client, char *topic, uint8_t* data, unsigned int len, unsigned int qos, unsigned int timeout_ms, MQTT_SYNC_CB sync_cb,void *pdata)
{
    PR_INFO("publish async topic %s qos %d", topic, qos);
    int msgid = mqtt_client_publish(client->mqtt_handler, topic, data, len, qos);
    if (msgid > 0 && qos > 0) {
        custom_message_t *msg = (custom_message_t *)tal_malloc(sizeof(custom_message_t));
        msg->msgid = msgid;
        msg->qos = qos;
        msg->timeout = tal_time_get_posix() + timeout_ms==0?MATOP_TIMEOUT_MS_DEFAULT:timeout_ms;
        msg->topic = mm_strdup(topic);
        msg->payload_len = 0;   // dont support retransmit
        msg->payload = NULL;    // dont support retransimit
        msg->is_sync = FALSE;
        msg->sync_cb = sync_cb;        
        msg->user_data = pdata;
        tuya_list_add_tail(&msg->node , &client->pubmsglist);        
    }

    return msgid > 0 ? OPRT_OK : OPRT_COM_ERROR;
}

/**
 * @brief a default sync callback, will push a semaphore when recvice the ack or timeout
 *
 * @param[in/out] rt
 * @param[in/out] prv_data
 * @return static
 */
static void __custom_mqtt_sync_cb(int rt, void *prv_data)
{
    custom_message_t *msg = (custom_message_t *)prv_data;
    if (msg->user_data) {
        custom_mqtt_sync_t *sync = (custom_mqtt_sync_t *) msg->user_data;
        PR_DEBUG("custom sync cb rt: %d", rt);
        sync->rt = rt;
        tal_semaphore_post(sync->sem_handle);
    }
}

/**
 * @brief publish data to topic sync
 *
 * @param[in] topic : dest topic
 * @param[in] data : publish data
 * @param[in] len : publish data len
 * @param[in] qos : publish qos
 * @param[in] timeout : timeout in seconds, works when qos > 0
 *
 * @note: if qos > 0, retransmit should be done in cb_pub_inform
 *
 * @return OPERATE_RET
 */
OPERATE_RET __custom_mqtt_publish_sync(MQTT_CLINT_LUA_T *client, char *topic, uint8_t *data, unsigned int len, unsigned int qos, unsigned int timeout_ms)
{
    PR_INFO("publish sync topic %s qos %d", topic, qos);
    OPERATE_RET rt = OPRT_OK;
    int msgid = mqtt_client_publish(client->mqtt_handler, topic, data, len, qos);
    if (msgid > 0) {
        custom_message_t *msg = (custom_message_t *)tal_malloc(sizeof(custom_message_t));        
        TUYA_CHECK_NULL_RETURN(msg, OPRT_MALLOC_FAILED);
        msg->msgid = msgid;
        msg->qos = qos;
        msg->timeout = tal_time_get_posix() + timeout_ms==0?MATOP_TIMEOUT_MS_DEFAULT:timeout_ms;
        msg->topic = mm_strdup(topic);
        msg->payload_len = 0;   // dont support retransmit
        msg->payload = NULL;    // dont support retransimit

        // sync callback
        custom_mqtt_sync_t *sync = (custom_mqtt_sync_t *)tal_malloc(sizeof(custom_mqtt_sync_t));
        if (!sync) {
            tal_free(msg);
            return OPRT_MALLOC_FAILED;
        }
        sync->rt = OPRT_COM_ERROR;
        if (OPRT_OK != tal_semaphore_create_init(&(sync->sem_handle), 0, 10)) {
            tal_free(sync);
            tal_free(msg);
            return OPRT_OS_ADAPTER_SEM_CREAT_FAILED;
        }

        msg->is_sync = TRUE;
        msg->sync_cb = __custom_mqtt_sync_cb;        
        msg->user_data = sync;
        tuya_list_add_tail(&msg->node , &client->pubmsglist);

        // wait for sync callback
        tal_semaphore_wait_forever(sync->sem_handle);
        PR_DEBUG("pub finish:%d", sync->rt);
        rt = sync->rt; 
        tal_semaphore_release(sync->sem_handle);
        tal_free(sync);
    } else {
        rt = OPRT_COM_ERROR;
    }

    return rt;
}

OPERATE_RET __custom_mqtt_client_subscribe(MQTT_CLINT_LUA_T *client, const char *topic, unsigned int qos, unsigned int timeout_ms)
{
    PR_INFO("Subscribe topic %s qos %d", topic, qos);
    uint16_t msgid = mqtt_client_subscribe(client->mqtt_handler, topic, qos);
    if (msgid != 0 && qos > 0) {
        custom_message_t *msg = (custom_message_t *)tal_malloc(sizeof(custom_message_t));
        msg->msgid = msgid;
        msg->qos = qos;
        msg->timeout = tal_time_get_posix() + timeout_ms==0?MATOP_TIMEOUT_MS_DEFAULT:timeout_ms;
        msg->topic = mm_strdup(topic);
        msg->payload_len = 0;   // dont support retransmit
        msg->payload = NULL;    // dont support retransimit
        msg->is_sync = FALSE;
        msg->sync_cb = NULL;        
        msg->user_data = NULL;
        tuya_list_add_tail(&msg->node , &client->submsglist);        
    }

    return msgid > 0 ? OPRT_OK : OPRT_COM_ERROR;
}

OPERATE_RET __custom_mqtt_client_unsubscribe(MQTT_CLINT_LUA_T *client, const char *topic, unsigned int qos, unsigned int timeout_ms)
{
    PR_INFO("UnSubscribe topic %s qos %d", topic, qos);
    uint16_t msgid = mqtt_client_unsubscribe(client->mqtt_handler, topic, qos);
    if (msgid != 0 && qos > 0) {
        custom_message_t *msg = (custom_message_t *)tal_malloc(sizeof(custom_message_t));
        msg->msgid = msgid;
        msg->qos = qos;
        msg->timeout = tal_time_get_posix() + timeout_ms==0?MATOP_TIMEOUT_MS_DEFAULT:timeout_ms;
        msg->topic = mm_strdup(topic);
        msg->payload_len = 0;   // dont support retransmit
        msg->payload = NULL;    // dont support retransimit
        msg->is_sync = FALSE;
        msg->sync_cb = NULL;        
        msg->user_data = NULL;
        tuya_list_add_tail(&msg->node , &client->submsglist);        
    }

    return msgid > 0 ? OPRT_OK : OPRT_COM_ERROR;
}

static char *__get_opt_string(lua_State *L, int index, const char *key)
{
    lua_getfield(L, index, key);  // 从 Lua 表中获取字段

    const char *value = lua_tostring(L, -1);  // 获取栈顶的字符串
    char *result = NULL;

    if (value) {
        // 计算字符串长度并分配内存
        size_t len = strlen(value) + 1; // +1 是为了包含字符串结束符 '\0'
        result = ( char *)tal_malloc(len); // 使用自定义内存分配函数

        if (result) {
            memcpy((void *)result, value, len); // 复制字符串内容
        }
    }

    lua_pop(L, 1);  // 弹出栈顶的字符串
    return result;  // 返回指向新分配字符串的指针
}

static int __get_opt_integer(lua_State *L, int index, const char *key)
{
    lua_getfield(L, index, key);  // 获取键对应的值并推入栈
    int value = lua_tointeger(L, -1);  // 获取栈顶的整数值
    lua_pop(L, 1);  // 弹出栈顶的整数值
    return value;  // 返回整数值
}

static int lua_mqttc_reg_on(lua_State *L)
{
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);
    const char *name = luaL_checkstring(L, 2);
    int ref;

    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3); 
    ref = luaL_ref(L, LUA_REGISTRYINDEX); 
    if (!strcmp(name, "conack"))
        client->on_conack_ref = ref;
    else if (!strcmp(name, "disconack"))
        client->on_disconack_ref = ref;
    else if (!strcmp(name, "puback"))
        client->on_puback_ref = ref;
    else if (!strcmp(name, "suback"))
        client->on_suback_ref = ref;
    else if (!strcmp(name, "unsuback"))
        client->on_unsuback_ref = ref;    
    else if (!strcmp(name, "message"))
        client->on_message_ref = ref;  
    else if (!strcmp(name, "publish"))
        client->on_publish_ref = ref;
    else
        luaL_argcheck(L, false, 2, "available event name: conack disconack puback suback unsuback");

    lua_pushfstring(L, "reg %s success", name);
    return 1;
}

static int lua_mqttc_ca(lua_State *L)
{
    // get client 
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);
    
    // get ca
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    client->ca.ca = (char *)tal_malloc(len);
    if (!client->ca.ca) {
        lua_pushfstring(L, "set ca malloc failed");
        return 1;
    }

    memcpy(client->ca.ca, str, len);
    client->ca.ca_len = len;
    lua_pushfstring(L, "set ca success");
    return 1;
}

static int lua_mqttc_return_code_string(lua_State *L)
{   
	int code = luaL_checkinteger(L, -1);
    switch (code) {
        case MQTT_STATUS_SUCCESS:
            lua_pushstring(L,"success");
            break;
        case MQTT_STATUS_INVALID_PARAM:
            lua_pushstring(L,"invalid parameter");
            break;
        case MQTT_STATUS_CONNECT_FAILED:
            lua_pushstring(L,"connect failed");
        break;
        case MQTT_STATUS_NOT_AUTHORIZED:
            lua_pushstring(L,"not authorized");
        break;
        case MQTT_STATUS_NETWORK_INIT_FAILED:
            lua_pushstring(L,"network init failed");
        break;
        case MQTT_STATUS_NETWORK_CONNECT_FAILED:
            lua_pushstring(L,"network connect failed");
        break;
        case MQTT_STATUS_NETWORK_TIMEOUT:
            lua_pushstring(L,"network timeout");
        break;
    default:
        lua_pushfstring(L, "Unknown return code: %d", code);
    }
	return 1;
}

/**
 * @brief connect to mqtt server
 * 
 * @param L : Lua engin
 * @return int 
 * 
 * @example 
 *          c = mqtt.new() 
 *          --c.ca("xxxx") -- if need ca
 *          c:on("conack", function () print("lua recv connect ack") end)
 *          c:on("disconack", function () print("lua recv dis-connect ack") end)
 *          c:connect({host="broker.emqx.io", port=1883, client_id="tuya-open-sdk-for-device-01", user_name="emqx", passwd="public"})
 */
static int lua_mqttc_connect(lua_State *L)
{
    // get client 
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);
    if (client->connected) {
        lua_pushfstring(L, "mqtt connected!");
        return 1;        
    }

    // get connect info
    luaL_checktype(L, 2, LUA_TTABLE);
    client->host = __get_opt_string(L, 2, "host");
    client->port = __get_opt_integer(L, 2, "port");
    client->signature.client_id = __get_opt_string(L, 2, "client_id");
    client->signature.user_name = __get_opt_string(L, 2, "user_name");
    client->signature.passwd = __get_opt_string(L, 2, "passwd");
    // client->keep_alive = __get_opt_integer(L, 2, "keep_alive");

    // 检查参数有效性
    if (client->host == NULL || client->port < 0 || client->port > 65535 || client->signature.client_id == NULL) {
        lua_pushnil(L);
        lua_pushfstring(L, "invalid connect options");
        return 2;
    }

    // start mqtt client
    lua_mqtt_client_set_state(client, STATE_MQTT_INIT);
    lua_pushfstring(L, "mqtt connecting...");
    return 1;
}

/**
 * @brief subscribe a topic after connect
 * 
 * @param L 
 * @return int 
 * 
 * @example
 *          c:on("suback", function () print("lua recv subscibe ack") end)
 *          c:subscribe("test", 0) 
 *          c:subscribe("test1", 1)
 */
static int lua_mqttc_subscribe(lua_State *L)
{
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);
    if (!client->connected) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "mqt not connected");
        return 2;
    }

    // subscribe
    const char *topic = luaL_checkstring(L, 2);
    uint8_t qos = luaL_checkinteger(L, 3);
    if (OPRT_OK == __custom_mqtt_client_subscribe(client, topic, qos, 0)) {
        lua_pushboolean(L, true); // success
        lua_pushstring(L, "mqtt subscribe success");
    } else {
        lua_pushboolean(L, false); // failed
        lua_pushstring(L, "mqtt subscribe failed");
    }

    return 2;
}

/**
 * @brief unsubscribe a topic after subscribe
 * 
 * @param L 
 * @return int 
 * 
 * @example
 *          c:on("unsuback", function () print("lua recv unsubscibe ack") end)
 *          c:unsubscribe("test", 0)
 *          c:unsubscribe("test1", 1)
 */
static int lua_mqttc_unsubscribe(lua_State *L)
{
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);
    if (!client->connected) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "mqtt not connected");
        return 2;
    }

    // subscribe
    const char *topic = luaL_checkstring(L, 2);
    uint8_t qos = luaL_checkinteger(L, 3);
    if (OPRT_OK == __custom_mqtt_client_unsubscribe(client, topic, qos, 0)) {
        lua_pushboolean(L, true); // success
        lua_pushstring(L, "mqtt unsubscribe success");
    } else {
        lua_pushboolean(L, false); // failed
        lua_pushstring(L, "mqtt ubsubscribe failed");
    }

    return 2;
}

static void __on_publish(OPERATE_RET op_ret, void *pri_data)
{
    PR_DEBUG("publish result: %d", op_ret);
    custom_message_t *msg = (custom_message_t*)pri_data;
    if (msg->user_data) {
        MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)msg->user_data;

        lua_State *L = client->L;
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_publish_ref);
        if (lua_isfunction(L, -1))
        {
            lua_pushinteger(L, op_ret);
            lua_pushlstring(L, msg->topic, strlen(msg->topic)+1);
            lua_pushlstring(L, msg->payload, msg->payload_len);
            if (lua_pcall(L,3,0,0) != LUA_OK) {
                PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }
        else {
            lua_pop(L, 1);
        }
    }
}

/**
 * @brief publish to a topic after connect to server
 * 
 * @param L 
 * @return int 
 * 
 * @example
 *          c:on("puback", function () print("lua recv publish message") end)
 *          c:on("publish", function () print("lua process publish") end)
 *          c:publish("test2", "a publish example") -- qos=0, async publish
 *          c:publish("test2", "another publish example", {qos=1}) -- qos=1, async publish
 *          c:publish("test2", "another publish example", {qos=1}) -- qos=1, with a publish callback async publish
 *          c:publish("test2", "another publish example", {qos=1, sync=1}) -- qos=1, sync publish
 *          c:publish("test2", "another publish example", {qos=1, timeout_ms=5000}) -- qos=1, sync publish, with 5000ms timeout(default is 8000ms)
 */
static int lua_mqttc_publish(lua_State *L)
{
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);
    const char *topic;
    size_t payloadlen;
    const char *payload;
    int qos = 0;
    int timeout_ms = 0;
    int is_sync = 0;
    int ref = 0;
    int ret = 0;

    // skip if not connected
    if (!client->connected) {
		lua_pushboolean(L, false);
		lua_pushstring(L, "not connected");
		return 2;
	}

    topic = luaL_checkstring(L, 2);
    payload = luaL_checklstring(L, 3, &payloadlen);
    if (!lua_isnone(L, 4) && !lua_isnil(L, 4)) {
        luaL_checktype(L, 4, LUA_TTABLE);

        // qos, default is 0
        lua_getfield(L, 4, "qos");
        if (!lua_isnil(L, -1))
            qos = lua_tointeger(L, -1);

        // timeout in ms, default is 5000
        lua_getfield(L, 4, "timeout_ms");
        if (!lua_isnil(L, -1))
            timeout_ms = lua_tointeger(L, -1);

        // sync or async, default is async
        lua_getfield(L, 4, "sync");
        if (!lua_isnil(L, -1))
           is_sync = lua_tointeger(L, -1); 
    }

    if (is_sync) {
        ret = __custom_mqtt_publish_sync(client, (char*)topic, (uint8_t*)payload, payloadlen, qos, timeout_ms);
    } else {
        ret = __custom_mqtt_publish_async(client, (char*)topic, (uint8_t*)payload, payloadlen, qos, timeout_ms, __on_publish, client);
    }
    lua_pushinteger(L, ret);
    return 1;
}

/**
 * @brief mqtt gc, release the resoure
 * 
 * @param L 
 * @return int 
 */
static int lua_mqttc_gc(lua_State *L)
{
    MQTT_CLINT_LUA_T *client = luaL_checkudata(L, 1, MQTTC_MT);

    // callbacks
    if (client->on_conack_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_conack_ref);
    if (client->on_message_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_message_ref);
    if (client->on_puback_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_puback_ref);
    if (client->on_disconack_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_disconack_ref);
    if (client->on_suback_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_suback_ref);
    if (client->on_unsuback_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_unsuback_ref);
    if (client->on_publish_ref != LUA_NOREF)
        luaL_unref(client->L, LUA_REGISTRYINDEX,client->on_publish_ref);

    // stop mqtt client and exit, will free client
    lua_mqtt_client_set_state(client, STATE_MQTT_STOP);
    return 0;
}

/**
 * @brief create a new mqtt client
 * 
 * @param L 
 * @return int t
 * 
 * @example
 *      c = mqtt.new()  
 */
static int lua_mqttc_new(lua_State *L)
{
    MQTT_CLINT_LUA_T *mqtt_client = (MQTT_CLINT_LUA_T *)lua_newuserdata(L, sizeof(MQTT_CLINT_LUA_T));
    memset(mqtt_client, 0, sizeof(0));
    mqtt_client->inited = FALSE;
    mqtt_client->connected = FALSE;
    mqtt_client->mqtt_handler = NULL;
    mqtt_client->on_conack_ref = LUA_NOREF;
    mqtt_client->on_disconack_ref = LUA_NOREF;
    mqtt_client->on_puback_ref = LUA_NOREF;
    mqtt_client->on_suback_ref = LUA_NOREF;
    mqtt_client->on_unsuback_ref = LUA_NOREF;
    mqtt_client->on_message_ref = LUA_NOREF;
    mqtt_client->on_publish_ref = LUA_NOREF;
    mqtt_client->L = L;
    INIT_LIST_HEAD(&mqtt_client->pubmsglist);
    INIT_LIST_HEAD(&mqtt_client->submsglist);

    THREAD_CFG_T thread_cfg = {.priority = THREAD_PRIO_3, .stackDepth = 4096, .thrdname = "lua-mqttc"};
    if (OPRT_OK != tal_thread_create_and_start(&mqtt_client->thread_handler, NULL, NULL, lua_mqtt_client_thread, mqtt_client, &thread_cfg)){
        tal_free(mqtt_client);
        lua_pushstring(L, "new mqtt client failed");
        return 1;
    } 

    // return mqtt client
	luaL_getmetatable(L, MQTTC_MT);
	lua_setmetatable(L, -2);
    return 1;
}

static int lua_mqttc_version(lua_State *L)
{
    lua_pushstring(L, "3.1.0");
    return 1;
}

static const luaL_Reg mqttc_meta[] = {
    {"on", lua_mqttc_reg_on},
    {"ca", lua_mqttc_ca},
    {"connect", lua_mqttc_connect},
    {"publish", lua_mqttc_publish},
    {"subscribe", lua_mqttc_subscribe},
    {"unsubscribe", lua_mqttc_unsubscribe},
    {"__gc", lua_mqttc_gc},
	{NULL, NULL}
};

static const luaL_Reg mqttc_fun[] = {
    {"new", lua_mqttc_new},
    {"version", lua_mqttc_version},
    {"return_code_string", lua_mqttc_return_code_string},
    {NULL, NULL}
};

int luaopen_mqttc(lua_State *L)
{
    /* metatable.__index = metatable */
    luaL_newmetatable(L, MQTTC_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, mqttc_meta, 0);

    lua_newtable(L);
    luaL_setfuncs(L, mqttc_fun, 0);

    return 1;
}
