/**
 * @file mqtt_bind.c
 * @brief Implementation of MQTT binding process for Tuya devices.
 *
 * This file contains the implementation of the MQTT binding process, which
 * includes initializing the MQTT service, connecting to the Tuya MQTT broker,
 * and handling the various states of the MQTT binding lifecycle. It utilizes
 * the Tuya IoT SDK to facilitate secure and reliable communication between the
 * device and the Tuya cloud platform.
 *
 * The binding process is crucial for devices to establish a secure MQTT
 * connection with the Tuya cloud, enabling them to send and receive messages
 * for device control, status updates, and other IoT functionalities.
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_config_defaults.h"
#include "tuya_error_code.h"
#include "tuya_iot.h"
#include "mqtt_service.h"
#include "cJSON.h"
#include "tal_api.h"
#include "netmgr.h"
#include "lua-mqttc.h"
#include "mqtt_client_interface.h"
#include "core_mqtt_config.h"
// #include "core_mqtt.h"

#define MQTTC_MT "qmttc"
#define CONVERT_HANDLE_TO_CLIENT(ptr, type, member)   ({(type *)( (char *)ptr - OFFSOF(type,member) );}) 

static void __proc_recv_ack(LIST_HEAD *head, int msgid) 
{
    // remove msg in list when msgid matched
    struct tuya_list_head * p = NULL;
    struct tuya_list_head * n = NULL;
    custom_message_t * msg = NULL;
    tuya_list_for_each_safe(p, n, head) {
        msg = tuya_list_entry(p, custom_message_t, node);
        if (msg->msgid == msgid) {
            tuya_list_del(&msg->node);
            if (msg->sync_cb) {
                msg->sync_cb(OPRT_OK, msg);
            }

            if (msg->payload) {
                tal_free(msg->payload);
            }
            tal_free(msg->topic);
            tal_free(msg);
            break;
        }
    }

    return;
}

static void __mqtt_client_connected_cb(MQTT_HANDLE handle, void *userdata)
{
    PR_INFO("mqtt client connected!");
    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)userdata;
    if (client == NULL || client->L == NULL) {
        return ;
    }

    // get mqtt connected ack
    client->connected = TRUE;
    lua_State *L = client->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_conack_ref);
    }

    // if connect ack callback is valid 
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
	} else {
        lua_pushboolean(L,1);
        lua_pushstring(L,"custom mqtt connected");
        if (lua_pcall(L,2,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

static void __mqtt_client_disconnected_cb(MQTT_HANDLE handle, void *userdata)
{
    PR_INFO("mqtt client disconnected!");
    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)userdata;

    if (client == NULL || client->L == NULL) {
        return ;
    }

    // get mqtt disconnected ack
    client->connected = FALSE;
    lua_State *L = client->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_disconack_ref);
    }

    // if disconnect ack callback is valid 
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
	} else {
        lua_pushboolean(L,1);
        lua_pushstring(L,"custom mqtt disconnected");
        if (lua_pcall(L,2,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

static void __mqtt_client_message_cb(MQTT_HANDLE handle, uint16_t msgid, const mqtt_client_message_t *msg, void *userdata)
{
    PR_DEBUG("recv message TopicName:%s, payload len:%d", msg->topic, msg->length);
    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)userdata;

    if (client == NULL || client->L == NULL) {
        return ;
    }

    // get mqtt disconnected ack
    lua_State *L = client->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_message_ref);
    }

    // if disconnect ack callback is valid 
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
	} else {
        // lua_pushboolean(L,1);
        lua_pushstring(L, msg->topic);
        lua_pushinteger(L, msg->qos);
        lua_pushinteger(L, msg->length);
        lua_pushlightuserdata(L, msg->payload);
        lua_pushlightuserdata(L, userdata);
        // lua_pushstring(L,"custom mqtt message");
        if (lua_pcall(L,5,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }    
}

static void __mqtt_client_puback_cb(MQTT_HANDLE handle, uint16_t msgid, void *userdata)
{
    PR_DEBUG("PUBACK successed ID:%d", msgid);
    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)userdata;

    if (client == NULL || client->L == NULL) {
        return ;
    }

    // get mqtt disconnected ack
    lua_State *L = client->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_puback_ref);
    }

    // if disconnect ack callback is valid 
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
	} else {
        lua_pushboolean(L,1);
        lua_pushinteger(L,msgid);
        lua_pushlightuserdata(L,userdata);
        if (lua_pcall(L,3,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } 

    __proc_recv_ack(&client->pubmsglist, msgid);
}

static void __mqtt_client_subscribed_cb(MQTT_HANDLE handle, uint16_t msgid, void *userdata)
{
    PR_DEBUG("Subscribe successed ID:%d", msgid);
    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)userdata;

    if (client == NULL || client->L == NULL) {
        return ;
    }

    // get mqtt disconnected ack
    lua_State *L = client->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_suback_ref);
    }

    // if disconnect ack callback is valid 
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
	} else {
        lua_pushboolean(L,1);
        lua_pushinteger(L,msgid);
        lua_pushlightuserdata(L,userdata);
        if (lua_pcall(L,3,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } 

    __proc_recv_ack(&client->submsglist, msgid);
}

static void __mqtt_client_unsubscribed_cb(MQTT_HANDLE handle, uint16_t msgid, void *userdata)
{
    PR_DEBUG("UnSubscribe successed ID:%d", msgid);
    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)userdata;

    if (client == NULL || client->L == NULL) {
        return ;
    }

    // get mqtt disconnected ack
    lua_State *L = client->L;
    if (L) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, client->on_unsuback_ref);
    }

    // if disconnect ack callback is valid 
	if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
	} else {
        lua_pushboolean(L,1);
        lua_pushinteger(L,msgid);
        lua_pushlightuserdata(L,userdata);
        if (lua_pcall(L,3,0,0) != LUA_OK) {
            PR_ERR( "Error calling Lua function: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } 
}

/**
 * @brief custom mqtt client init
 *
 * @param[in] client : the mqtt client
 * @return int mqtt init status
 */
static int __mqtt_custom_init(MQTT_CLINT_LUA_T *client)
{
    int rt = OPRT_OK;
    PR_DEBUG("custom connect MQTT URL:%s Port:%d", client->host, client->port);
    PR_DEBUG("custom MQTT client_id:%s", client->signature.client_id);
    PR_DEBUG("custom MQTT user_name:%s", client->signature.user_name);
    PR_DEBUG("custom MQTT passwd:%s", client->signature.passwd);
    // PR_DEBUG("custom MQTT subcribe topic:%s", topic);

    /* MQTT Client init */
    client->mqtt_handler = mqtt_client_new();
    const mqtt_client_config_t mqtt_config = {.cacert = client->ca.ca,
                                              .cacert_len = client->ca.ca_len,
                                              .host = client->host,
                                              .port = client->port,
                                              .keepalive = client->keep_alive > 0 ? client->keep_alive : MQTT_KEEPALIVE_INTERVALIN,
                                              .timeout_ms = MATOP_TIMEOUT_MS_DEFAULT,
                                              .clientid = client->signature.client_id,
                                              .username = client->signature.user_name,
                                              .password = client->signature.passwd,
                                              .on_connected = __mqtt_client_connected_cb,
                                              .on_disconnected = __mqtt_client_disconnected_cb,
                                              .on_message = __mqtt_client_message_cb,
                                              .on_subscribed = __mqtt_client_subscribed_cb,
                                              .on_unsubscribed = __mqtt_client_unsubscribed_cb,
                                              .on_published = __mqtt_client_puback_cb,
                                              .userdata = client};
    // init mqtt client
    return mqtt_client_init(client->mqtt_handler, &mqtt_config);
}

/**
 * @brief custom mqtt client connect
 * 
 * @param client : the mqtt client
 * @return int mqtt connect status
 */
static int __mqtt_custom_connect(MQTT_CLINT_LUA_T *client)
{
    // connect to mqtt server
    return mqtt_client_connect(client->mqtt_handler);
}

/**
 * @brief 
 * 
 * @param client 
 * @return int 
 */
static int __mqtt_custom_loop(MQTT_CLINT_LUA_T *client)
{
    int rt = OPRT_OK;
    mqtt_client_status_t mqtt_status;

    // check connection
    if (client->connected == FALSE) {
        mqtt_status = __mqtt_custom_connect(client);
        if (mqtt_status != MQTT_STATUS_SUCCESS) {
            uint16_t nextRetryBackOff = 0U;
            if (BackoffAlgorithm_GetNextBackoff(&client->backoff_algorithm, rand(), &nextRetryBackOff) ==
                BackoffAlgorithmSuccess) {
                PR_WARN("Connection to the MQTT server failed. Retrying "
                        "connection after %hu ms backoff.",
                        (unsigned short)nextRetryBackOff);
                tal_system_sleep(nextRetryBackOff);
                return rt;
            }
        }
        return rt;
    }

    /* LOCK */
    /* publish async process */
    struct tuya_list_head * p = NULL;
    struct tuya_list_head * n = NULL;
    custom_message_t * msg = NULL;
    tuya_list_for_each_safe(p, n, &client->pubmsglist) {
        msg = tuya_list_entry(p, custom_message_t, node);
        if (msg->timeout <= tal_time_get_posix()) {
            tuya_list_del(&msg->node);
            if (msg->sync_cb) {
                msg->sync_cb(OPRT_TIMEOUT, msg);
            }
            if (msg->payload) {
                tal_free(msg->payload);
            }
            tal_free(msg->topic);
            tal_free(msg);
            continue;
        }

        // // resend
        // if (msg->msgid <= 0) {
        //     msg->msgid = mqtt_client_publish(client->mqtt_handler, msg->topic, msg->payload, msg->payload_length, 1);
        // }
    }
    /* UNLOCK */

    /* yield */
    return mqtt_client_yield(client->mqtt_handler);
}

/**
 * @brief custom mqtt client stop
 *
 * @param[in] client : the mqtt client
 * @return OPRT_OK on success, others on failed
 */
OPERATE_RET __mqtt_custom_exit(MQTT_CLINT_LUA_T *client)
{
    PR_DEBUG("stop custom mqtt");

    if (client->inited) {
        client->inited = FALSE;
        if (client->connected) {
            mqtt_client_disconnect(client->mqtt_handler);
            client->connected = FALSE;
        }
        
        // deinit mqtt handle
        mqtt_client_deinit(client->mqtt_handler);
        tal_free(client->mqtt_handler);
        client->mqtt_handler = NULL;

        // free ca 
        if (client->ca.ca) {
            tal_free(client->ca.ca);
            client->ca.ca = NULL;
            client->ca.ca_len = 0;
        }

        // free signature
        if (client->signature.client_id) {
            tal_free(client->signature.client_id);
            if (client->signature.user_name)
                tal_free(client->signature.user_name);
            if (client->signature.passwd)
                tal_free(client->signature.passwd);

            client->signature.client_id = NULL;
            client->signature.user_name = NULL;
            client->signature.passwd = NULL;
        }

        // free host
        if (client->host) {
            tal_free(client->host);
            client->host = NULL;
            client->port = 0;
        }
    }

    return OPRT_OK;
}

/**
 * @brief user defined network check callback, it will check the network every
 * 1sec, in this demo it alwasy return ture due to it's a wired demo
 *
 * @return true
 * @return false
 */
static bool __network_check(void)
{
    netmgr_status_e status = NETMGR_LINK_DOWN;
    netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_STATUS, &status);
    return status == NETMGR_LINK_DOWN ? false : true;
}

/**
 * @brief This function is a thread function that retrieves the MQTT bind token.
 *
 * @param args Pointer to the arguments passed to the thread.
 */
void lua_mqtt_client_thread(void *args)
{
    OPERATE_RET rt = OPRT_OK;

    MQTT_CLINT_LUA_T *client = (MQTT_CLINT_LUA_T *)args;

    client->inited = TRUE;
    client->state = STATE_MQTT_IDLE;
    while (client->state != STATE_MQTT_EXIT) {
        switch (client->state) {
        case STATE_MQTT_IDLE:
            tal_system_sleep(500);  // wait for connect
            break;
        case STATE_MQTT_INIT:
            rt = __mqtt_custom_init(client);
            if (OPRT_OK != rt) {
                PR_ERR("mqtt init fail:%d, retry..", rt);
                tal_system_sleep(1000);
                break;
            }
            client->state = STATE_MQTT_CONNECT; 
            break;
        case STATE_MQTT_CONNECT:
            // network check
            if (__network_check()) {                
                tal_system_sleep(1000);
                break;
            }

            // connect to server
            rt = __mqtt_custom_connect(client);
            if (OPRT_OK != rt) {
                PR_ERR("mqtt connect fail:%d, retry..", rt);
                tal_system_sleep(1000);
                break;
            }

            client->state = STATE_MQTT_COMPLETE;
            break;

        case STATE_MQTT_COMPLETE:
            __mqtt_custom_loop(client);
            break;

        case STATE_MQTT_STOP:
            PR_DEBUG("recv STATE_MQTT_EXIT");
            __mqtt_custom_exit(client);
            client->state = STATE_MQTT_EXIT;
            break;

        default:
            PR_ERR("state error:%d", client->state);
            break;
        }
    }

    if (client->thread_handler != NULL) {
        tal_thread_delete(client->thread_handler);
        client->thread_handler = NULL;
    }

    tal_free(client);
}

/**
 * @brief This function was used for change the MQTT client state
 * 
 * @param state 
 */
void lua_mqtt_client_set_state(MQTT_CLINT_LUA_T *client, custom_mqtt_state_t state)
{
    if (client && client->inited == TRUE) {
        client->state = state;
    }
}