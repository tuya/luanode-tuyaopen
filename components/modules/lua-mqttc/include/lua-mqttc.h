/**
 * @file luamqtt.h
 * @brief
 * @version 0.1
 * @date 2024-10-16
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 * Permission is hereby granted, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), Under the premise of complying
 * with the license of the third-party open source software contained in the software,
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software.
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 */

#ifndef _LUAMQTT_H_
#define _LUAMQTT_H_


#include <lauxlib.h>
#include <lualib.h>
#include "tuya_list.h"
#include "backoff_algorithm.h"

typedef void* MQTT_HANDLE;
typedef void (*MQTT_SYNC_CB)(int rt, void *user_data);

typedef enum {
    STATE_MQTT_INIT,
    // STATE_MQTT_START,
    STATE_MQTT_NETWORK_CHECK,
    STATE_MQTT_CONNECT,
    STATE_MQTT_COMPLETE,
    STATE_MQTT_DISCONNECT,
    STATE_MQTT_STOP,
    STATE_MQTT_EXIT,
    STATE_MQTT_IDLE,
} custom_mqtt_state_t;

typedef struct {
    char * client_id;
    char * user_name;
    char * passwd;
} custom_signature_t;

typedef struct {
    int ca_len;
    char * ca;
} custom_ca_t;

typedef struct {
    int msgid;

    char *topic;
    uint8_t qos;
    int timeout;
    uint8_t *payload;
    int payload_len;

    // sync
    BOOL_T is_sync;
    void *user_data;
    MQTT_SYNC_CB sync_cb;

    LIST_HEAD node;
} custom_message_t;

typedef struct mqttc_client_lua {
    BOOL_T inited;
    BOOL_T connected;
    MQTT_HANDLE mqtt_handler;
    THREAD_HANDLE thread_handler;

    char *host;
    int port;
    int keep_alive;
    int timeout;
    custom_mqtt_state_t state;
    BackoffAlgorithmContext_t backoff_algorithm;
    custom_signature_t signature;
    custom_ca_t ca;

    LIST_HEAD submsglist;
    LIST_HEAD pubmsglist;

    lua_State *L;
    int on_conack_ref;
    int on_disconack_ref;
    int on_message_ref;
    int on_suback_ref;
    int on_puback_ref;
    int on_unsuback_ref;
    int on_publish_ref;
}MQTT_CLINT_LUA_T;

/**
 * @brief This function is a thread function that retrieves the MQTT bind token.
 *
 * @param args Pointer to the arguments passed to the thread.
 */
void lua_mqtt_client_thread(void *args);

/**
 * @brief This function was used for change the MQTT client state
 * 
 * @param state 
 */
void lua_mqtt_client_set_state(MQTT_CLINT_LUA_T *client, custom_mqtt_state_t state);
#endif /* LUAMQTT */

