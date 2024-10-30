
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
#include "tal_log.h"
#include "tuya_error_code.h"
#include "tal_security.h"
#include "tal_semaphore.h"
#include "tal_memory.h"
#include "tal_workq_service.h"
#include "tuya_svc_mqtt_client.h"
#include "luamqttc.h"

#define MQTTC_MT "qmttc"

typedef struct {
    BOOL_T inited;
    BOOL_T connected;
    MQTT_HANDLE mqtt_handler;
} custom_connect_t;

typedef struct {
    OPERATE_RET rt;
    SEM_HANDLE sem_handle;
} custom_mqtt_sync_t;

typedef struct {
    CHAR_T * client_id;
    CHAR_T * user_name;
    CHAR_T * passwd;
} custom_signature_t;

/**
 * @brief custom mqtt client
 *
 */
STATIC custom_connect_t s_custom_con = {0};
STATIC MQTT_CLINT_LUA_T *s_mqtt_client_lua = NULL;

/**
 * @brief quit custom mqtt client
 *
 * @param[in/out] data: should be NULL
 * @return VOID
 */
STATIC VOID __mqtt_custom_quit(VOID *data)
{
    PR_DEBUG("quit custom mqtt");

    if (s_custom_con.inited) {
        s_custom_con.inited = FALSE;
        s_custom_con.connected = FALSE;

        tuya_svc_mqtt_client_destroy(s_custom_con.mqtt_handler);
    }
}

/**
 * @brief custom mqtt client receive data
 *
 * @param[in] topic : received topic
 * @param[in] data : received data
 * @param[in] len : received data len
 * @return VOID
 */
STATIC VOID __mqtt_custom_recv(const CHAR_T *topic, IN BYTE_T *data, IN UINT_T len)
{
    // TBD...
    // custom receive DATA process

    if (s_mqtt_client_lua && s_mqtt_client_lua->L && s_mqtt_client_lua->on_message_ref != LUA_REFNIL) {
        lua_State *L = s_mqtt_client_lua->L;
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_mqtt_client_lua->on_message_ref);
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1); // 弹出非函数引用
            return;
        }
        lua_pushstring(L, topic);
        lua_pushlstring(L, (const char *)data, len);
        lua_pushinteger(L, len);
        if (lua_pcall(L,3,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

/**
 * @brief custom mqtt client connected callback
 *
 * @return VOID
 */
STATIC VOID __mqtt_custom_conn_cb(VOID)
{
    PR_DEBUG("custom mqtt connected");

    s_custom_con.connected = TRUE;

    if (s_mqtt_client_lua == NULL || s_mqtt_client_lua->L == NULL) {
        return ;
    }
    lua_State *L = s_mqtt_client_lua->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_mqtt_client_lua->on_conack_ref);
    }
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
	}
    else {
        lua_pushboolean(L,1);
        lua_pushstring(L,"custom mqtt connected");
        if (lua_pcall(L,2,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

/**
 * @brief custom mqtt client disconnected callback
 *
 * @return VOID
 */
STATIC VOID __mqtt_custom_disconn_cb(VOID)
{
    PR_DEBUG("custom mqtt disconnected");
    s_custom_con.connected = FALSE;
    if (s_mqtt_client_lua == NULL || s_mqtt_client_lua->L == NULL) {
        return ;
    }
    lua_State *L = s_mqtt_client_lua->L;
    s_custom_con.connected = TRUE;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_mqtt_client_lua->on_conack_ref);
    }
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
	}
    else {
        lua_pushinteger(L,-1);
        lua_pushstring(L,"mqtt disconnected");
        if (lua_pcall(L,2,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

/**
 * @brief custom mqtt client connect denyed callback
 *
 * @param[in] deny_times : time denyed
 * @return VOID
 */
STATIC VOID __mqtt_custom_conn_deny_cb(IN BYTE_T deny_times)
{
    PR_DEBUG("custom mqtt connect deny:%d", deny_times);
    s_custom_con.connected = FALSE;

    if (s_mqtt_client_lua == NULL || s_mqtt_client_lua->L == NULL) {
        return ;
    }
    lua_State *L = s_mqtt_client_lua->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_mqtt_client_lua->on_conack_ref);
    }
	if (!lua_isfunction(L, -1)) {
        return;
	}
    else {
        lua_pushinteger(L,-2);
        lua_pushlstring(L, "mqtt connect failed retry times=%d", deny_times);
        if (lua_pcall(L,2,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

/**
 * @brief custom mqtt client init
 *
 * @param[in] domain : the URL of the custom mqtt server
 * @param[in] port : the port numbed of the custom mqtt server
 * @return OPRT_OK on success, others on failed
 */
STATIC OPERATE_RET __mqtt_custom_init(CHAR_T *domain, UINT_T port, CHAR_T* topic, custom_signature_t *signature)
{
    OPERATE_RET rt = OPRT_OK;
    s_custom_con.connected = FALSE;
    PR_DEBUG("custom connect MQTT URL:%s Port:%d", domain, port);
    PR_DEBUG("custom MQTT client_id:%s", signature->client_id);
    PR_DEBUG("custom MQTT user_name:%s", signature->user_name);
    PR_DEBUG("custom MQTT passwd:%s", signature->passwd);
    PR_DEBUG("custom MQTT subcribe topic:%s", topic);

    mqtt_ctx_t mqtt_ctx = {0};
    mqtt_ctx.broker_domain = domain;
    mqtt_ctx.broker_port = port;
    mqtt_ctx.cb_data_recv = __mqtt_custom_recv;
    mqtt_ctx.client_id = signature->client_id;
    mqtt_ctx.user_name = signature->user_name;
    mqtt_ctx.passwd = signature->passwd;
    mqtt_ctx.subcribe_topic = topic;
    mqtt_ctx.heartbeat = 0;
    TUYA_CALL_ERR_GOTO(tuya_svc_mqtt_client_create(&mqtt_ctx, &s_custom_con.mqtt_handler), ERR_EXIT);
    TUYA_CALL_ERR_GOTO(tuya_svc_mqtt_client_register_cb(s_custom_con.mqtt_handler, __mqtt_custom_conn_cb, __mqtt_custom_disconn_cb, __mqtt_custom_conn_deny_cb), ERR_EXIT);
    TUYA_CALL_ERR_GOTO(tuya_svc_mqtt_client_start(s_custom_con.mqtt_handler), ERR_EXIT);

    return rt;
ERR_EXIT:
    PR_ERR("custom mqtt init failed, rt %d!", rt);
    tuya_svc_mqtt_client_destroy(s_custom_con.mqtt_handler);
    return rt;
}

/**
 * @brief custom mqtt client start
 *
 * @param[in] domain : the URL of the custom mqtt server
 * @param[in] port : the port numbed of the custom mqtt server
 * @return OPRT_OK on success, others on failed
 */
OPERATE_RET mqtt_custom_start(CHAR_T *domain, UINT_T port, CHAR_T* topic, custom_signature_t *signature)
{
    TUYA_CHECK_NULL_RETURN(domain, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(topic, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(signature, OPRT_INVALID_PARM);

    // psk mode cannot support custom mqtt client
    if (TUYA_SECURITY_LEVEL == 0) {
        return OPRT_NOT_SUPPORTED;
    }

    if (s_custom_con.inited) {
        return OPRT_OK;
    }

    OPERATE_RET rt = OPRT_OK;

    PR_DEBUG("start custom mqtt");
    s_custom_con.inited = TRUE;
    TUYA_CALL_ERR_GOTO(__mqtt_custom_init(domain, port, topic, signature), ERR_EXIT);
    return OPRT_OK;

ERR_EXIT:
    s_custom_con.inited = FALSE;
    return rt;
}

/**
 * @brief
 *
 * @return OPRT_OK on success, others on failed
 */
OPERATE_RET mqtt_custom_stop(VOID)
{
    if (!s_custom_con.inited) {
        return OPRT_OK;
    }

    PR_DEBUG("stop custom mqtt");
    return tal_workq_schedule(WORKQ_SYSTEM, __mqtt_custom_quit, NULL);
}

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
 * @return OPERATE_RET
 */
OPERATE_RET mqtt_custom_publish_async(CHAR_T *topic, BYTE_T* data, UINT_T len, UINT_T qos, UINT_T timeout, CB_MQTT_PUB_INFORM cb_pub_inform,VOID *pdata)
{
    OPERATE_RET rt = OPRT_OK;
    mqtt_msg_t msg = {0};
    msg.publish_topic = topic;
    msg.cb_pub_inform = cb_pub_inform;
    msg.data = data;
    msg.len = len;
    msg.qos = qos;
    msg.timeout = timeout;
    msg.ctx = pdata;
    TUYA_CALL_ERR_RETURN(tuya_svc_mqtt_client_publish(s_custom_con.mqtt_handler, &msg));

    return OPRT_OK;
}

/**
 * @brief
 *
 * @param[in/out] rt
 * @param[in/out] prv_data
 * @return STATIC
 */
STATIC VOID __custom_mqtt_sync_cb(OPERATE_RET rt, VOID *prv_data)
{
    custom_mqtt_sync_t *sync = (custom_mqtt_sync_t *)prv_data;
    PR_DEBUG("custom sync cb rt: %d", rt);
    sync->rt = rt;
    tal_semaphore_post(sync->sem_handle);
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
OPERATE_RET mqtt_custom_publish_sync(CHAR_T *topic, BYTE_T *data, UINT_T len, UINT_T qos, UINT_T timeout)
{
    OPERATE_RET rt = OPRT_OK;

    custom_mqtt_sync_t *sync = tal_malloc(SIZEOF(custom_mqtt_sync_t));
    TUYA_CHECK_NULL_RETURN(sync, OPRT_MALLOC_FAILED);
    sync->rt = OPRT_COM_ERROR;
    TUYA_CALL_ERR_GOTO(tal_semaphore_create_init(&(sync->sem_handle), 0, 10), SEM_ERR_EXIT);

    mqtt_msg_t msg = {0};
    msg.publish_topic = topic;
    msg.cb_pub_inform = __custom_mqtt_sync_cb;
    msg.ctx = sync;
    msg.qos = qos;
    msg.data = data;
    msg.len = len;
    msg.timeout = timeout;
    TUYA_CALL_ERR_GOTO(tuya_svc_mqtt_client_publish(s_custom_con.mqtt_handler, &msg), PUB_ERR_EXIT);

    tal_semaphore_wait_forever(sync->sem_handle);
    PR_DEBUG("pub finish:%d", sync->rt);
    rt = sync->rt;

PUB_ERR_EXIT:
    tal_semaphore_release(sync->sem_handle);

SEM_ERR_EXIT:
    if (sync)
        tal_free(sync);

    return rt;
}

OPERATE_RET custom_mqtt_client_subscribe(MQTT_HANDLE handle, CHAR_T **topics, BYTE_T cnt, CB_MQTT_DATA_RECV cb_data_recv)
{
    mqtt_subscribe_t sub;
    sub.topics = topics;
    sub.cnt = cnt;
    sub.cb_data_recv = cb_data_recv;

    return tuya_svc_mqtt_client_subscribe(handle, &sub);
}

OPERATE_RET custom_mqtt_client_unsubscribe(MQTT_HANDLE handle, CHAR_T **topics, BYTE_T cnt, CB_MQTT_DATA_RECV cb_data_recv)
{
    mqtt_subscribe_t sub;
    sub.topics = topics;
    sub.cnt = cnt;
    sub.cb_data_recv = cb_data_recv;

    return tuya_svc_mqtt_client_unsubscribe(handle, &sub);
}

static int lua_mqttc_reg_on(lua_State *L)
{
    MQTT_CLINT_LUA_T *cl = luaL_checkudata(L, 1, MQTTC_MT);
    const char *name = luaL_checkstring(L, 2);
    int ref;

    luaL_checktype(L, 3, LUA_TFUNCTION);
    // 将函数的引用存储到栈中
    lua_pushvalue(L, 3); // 将函数推到栈顶
    ref = luaL_ref(L, LUA_REGISTRYINDEX); // 创建引用
    if (!strcmp(name, "conack"))
        cl->on_conack_ref = ref;
    else if (!strcmp(name, "message"))
        cl->on_message_ref = ref;
    else if (!strcmp(name, "publish"))
        cl->on_publish_ref = ref;
    else
        luaL_argcheck(L, false, 2, "available event name: conack suback unsuback publish pingresp error close");

	return 0;
}
static int lua_mqttc_return_code_string(lua_State *L)
{
	int code = luaL_checkinteger(L, -1);
    switch (code) {
        case OPRT_OK:
            lua_pushstring(L,"success");
            break;
        case OPRT_INVALID_PARM:
            lua_pushstring(L,"invalid parameter");
            break;
        case OPRT_MID_MQTT_PUBLISH_TIMEOUT:
            lua_pushstring(L,"publish timeout");
        break;
        case OPRT_MALLOC_FAILED:
            lua_pushstring(L,"malloc failed");
        break;
    default:
        lua_pushfstring(L, "Unknown return code: %d", code);
    }
	return 1;
}

static char *get_opt_string(lua_State *L, int index, const char *key)
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


static int get_opt_integer(lua_State *L, int index, const char *key)
{
    lua_getfield(L, index, key);  // 获取键对应的值并推入栈
    int value = lua_tointeger(L, -1);  // 获取栈顶的整数值
    lua_pop(L, 1);  // 弹出栈顶的整数值
    return value;  // 返回整数值
}

/*
local connect_options = {
    host = "mqtt.example.com",  -- 替换为实际的主机名
    port = 1883,                 -- MQTT 默认端口
    client_id = "my_client_id",  -- 替换为实际的客户端 ID
    username = "my_username",     -- 替换为实际的用户名
    password = "my_password",     -- 替换为实际的密码
    keep_alive = 60,             -- 保持连接的时间

}

-- 假设 mqtt_client 是已经初始化的 MQTT 客户端 userdata
local mqtt_client = mqttc_new()  -- 使用您的 C 函数创建客户端

-- 调用 umqtt_lua_connect 函数
local success = mqtt_client:connect(connect_options)
 */
static int lua_mqttc_connect(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    char *host = get_opt_string(L, 1, "host");
    int port = get_opt_integer(L, 1, "port");
    char *topic = get_opt_string(L, 1, "topic");
    char *clientid = get_opt_string(L, 1, "client_id");
    char *username = get_opt_string(L, 1, "username");
    char *passwd = get_opt_string(L, 1, "password");
    int keep_alive = get_opt_integer(L, 1, "keep_alive");

    // 检查参数有效性
    if (host == NULL || port < 0 || port > 65535 || topic == NULL || clientid == NULL) {
        lua_pushnil(L);
        lua_pushfstring(L, "invalid connect options");
        return 2;
    }
    if (keep_alive < 0) {
        keep_alive = 60; // 设置默认值
    }

    custom_signature_t signature;
    signature.client_id = clientid;
    signature.user_name = username;
    signature.passwd = passwd;

    int ret = mqtt_custom_start(host, port, topic, &signature);

    tal_free(host);
    tal_free(clientid);
    if (topic)
        tal_free(topic);
    if (username)
        tal_free(username);
    if (passwd)
        tal_free(passwd);

    if (ret) {
        lua_pushnil(L);
        lua_pushfstring(L, "mqtt_custom_start() failed");
        return 2;
    }
    if (keep_alive > 0) {
        tuya_svc_mqtt_client_set_cfg(s_custom_con.mqtt_handler, MQTT_CFG_HEARTBEAT, &keep_alive);
    }
    lua_pushinteger(L, ret);
    return 1;
}



static int lua_mqttc_subscribe(lua_State *L)
{
    char **topics;
    int n = lua_gettop(L) - 1; // 获取参数数量
    int i;

    if (!s_custom_con.connected) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "not connected");
        return 2;
    }

    if (n < 1) {
        lua_pushboolean(L, true);
        return 1; // 没有主题订阅时返回成功
    }

    // 为 topics 分配内存，分配 n 个指针
    topics = (char **)tal_calloc(n, sizeof(char *));
    if (!topics) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "memory allocation failed");
        return 2; // 内存分配失败
    }

    for (i = 0; i < n; i++) {
        luaL_checktype(L, i + 2, LUA_TTABLE); // 检查每个主题是否是表

        lua_getfield(L, i + 2, "topic"); // 获取主题字段
        if (!lua_isstring(L, -1)) {
            luaL_argerror(L, i + 2, "topic field represented by a string expected, got nil");
        }

        // 分配并复制主题字符串
        topics[i] = mm_strdup(lua_tostring(L, -1)); // 使用 mm_strdup 复制字符串
        lua_pop(L, 1); // 弹出主题字符串
    }

    // 调用订阅函数
    custom_mqtt_client_subscribe(s_custom_con.mqtt_handler, topics, n, NULL);

    // 释放主题内存
    for (i = 0; i < n; i++) {
        tal_free(topics[i]); // 释放每个主题字符串
    }
    tal_free(topics); // 释放指针数组

    lua_pushboolean(L, true); // 返回成功
    return 1;
}


static  int lua_mqttc_unsubscribe(lua_State *L)
{
   char **topics;
    int n = lua_gettop(L) - 1; // 获取参数数量
    int i;

    if (!s_custom_con.connected) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "not connected");
        return 2;
    }

    if (n < 1) {
        lua_pushboolean(L, true);
        return 1; // 没有主题订阅时返回成功
    }

    // 为 topics 分配内存，分配 n 个指针
    topics = (char **)tal_calloc(n, sizeof(char *));
    if (!topics) {
        lua_pushinteger(L, OPRT_MALLOC_FAILED);
        PR_DEBUG("memory allocation failed");
        return 1; // 内存分配失败
    }

    for (i = 0; i < n; i++) {
        luaL_checktype(L, i + 2, LUA_TTABLE); // 检查每个主题是否是表

        lua_getfield(L, i + 2, "topic"); // 获取主题字段
        if (!lua_isstring(L, -1)) {
            luaL_argerror(L, i + 2, "topic field represented by a string expected, got nil");
        }

        // 分配并复制主题字符串
        topics[i] = mm_strdup(lua_tostring(L, -1)); // 使用 mm_strdup 复制字符串
        lua_pop(L, 1); // 弹出主题字符串
    }

    // 调用订阅函数
    int ret = custom_mqtt_client_unsubscribe(s_custom_con.mqtt_handler, topics, n, NULL);
    PR_DEBUG("unsubscribe result: %d", ret);
    // 释放主题内存
    for (i = 0; i < n; i++) {
        tal_free(topics[i]); // 释放每个主题字符串
    }
    tal_free(topics); // 释放指针数组

    lua_pushinteger(L, ret); // 返回成功
    return 1;
}

static  VOID on_publish(OPERATE_RET op_ret, VOID *ctx)
{
    MQTT_TOPIC_LUA_T *cl = (MQTT_TOPIC_LUA_T*)ctx;
    PR_DEBUG("publish result: %d", op_ret);

    if (cl == NULL) {
        return;
    }

    lua_State *L = cl->mqtt_client->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, cl->mqtt_client->on_publish_ref);
	if (lua_isfunction(L, -1))
    {
        lua_pushinteger(L, op_ret);
        lua_pushlstring(L, cl->topic, cl->topic_len);
        lua_pushlstring(L, cl->payload, cl->payload_len);
        if (lua_pcall(L,3,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    else {
        lua_pop(L, 1);
    }

}

static  int lua_mqttc_publish(lua_State *L)
{
    MQTT_CLINT_LUA_T *cl = luaL_checkudata(L, 1, MQTTC_MT);
    const char *topic;
    size_t payloadlen;
    const char *payload;
    int qos = 0;
    int ret = 0;

    if (!s_custom_con.connected) {
		lua_pushboolean(L, false);
		lua_pushstring(L, "not connected");
		return 2;
	}
    topic = luaL_checkstring(L, 2);
    payload = luaL_checklstring(L, 3, &payloadlen);

    if (!lua_isnone(L, 4) && !lua_isnil(L, 4)) {
        luaL_checktype(L, 4, LUA_TTABLE);

        lua_getfield(L, 4, "qos");
        if (!lua_isnil(L, -1))
            qos = lua_tointeger(L, -1);
    }
    if (!qos) {
        MQTT_TOPIC_LUA_T    *tp = tal_malloc(sizeof(MQTT_TOPIC_LUA_T));

        tp->topic = mm_strdup(topic);
        tp->topic_len = strlen(topic);
        tp->payload = mm_strdup(payload);
        tp->payload_len = payloadlen;
        tp->mqtt_client = cl;
        ret = mqtt_custom_publish_async(tp->topic, (BYTE_T*)tp->payload, payloadlen, qos,0,on_publish,tp);
    }
    else {
        ret = mqtt_custom_publish_async((CHAR_T*)topic,(BYTE_T*)payload, payloadlen, qos,0,NULL,NULL);
    }
    lua_pushinteger(L, ret);
    return 1;
}
static int lua_mqttc_gc(lua_State *L)
{
    MQTT_CLINT_LUA_T *cl = luaL_checkudata(L, 1, MQTTC_MT);
    if (cl) {
        mqtt_custom_stop();
        if (cl->on_conack_ref != LUA_NOREF)
            luaL_unref(cl->L, LUA_REGISTRYINDEX,cl->on_conack_ref);
        if (cl->on_message_ref != LUA_NOREF)
            luaL_unref(cl->L, LUA_REGISTRYINDEX,cl->on_message_ref);
        if (cl->on_publish_ref != LUA_NOREF)
            luaL_unref(cl->L, LUA_REGISTRYINDEX,cl->on_publish_ref);

    }
    return 0;
}
static int lua_mqttc_new(lua_State *L)
{
    if (s_mqtt_client_lua) {
        if (s_mqtt_client_lua->L) {
            lua_pushlightuserdata(L, s_mqtt_client_lua);
            return 1;
        }
    }
    s_mqtt_client_lua = lua_newuserdata(L, sizeof(MQTT_CLINT_LUA_T));
    s_mqtt_client_lua->on_conack_ref = LUA_NOREF;
    s_mqtt_client_lua->on_message_ref = LUA_NOREF;
    s_mqtt_client_lua->on_publish_ref = LUA_NOREF;
    s_mqtt_client_lua->L = L;
	luaL_getmetatable(L, MQTTC_MT);
	lua_setmetatable(L, -2);
     // 直接返回 userdata，而不是使用 light userdata
    return 1;
}

static int mqttc_lua_version(lua_State *L)
{
    lua_pushstring(L, "3.1.0");
    return 1;
}
static const luaL_Reg mqttc_meta[] = {

    {"on", lua_mqttc_reg_on},
    {"publish", lua_mqttc_publish},
    {"subscribe", lua_mqttc_subscribe},
    {"unsubscribe", lua_mqttc_unsubscribe},
    {"__gc", lua_mqttc_gc},
	{NULL, NULL}
};

static const luaL_Reg mqttc_fun[] = {
    {"new", lua_mqttc_new},
    {"connect", lua_mqttc_connect},
    {"version", mqttc_lua_version},
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
