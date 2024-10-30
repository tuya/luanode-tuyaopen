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

#ifndef LUAMQTTC
#define LUAMQTTC
#ifndef _LUAMQTT_H_
#define _LUAMQTT_H_


#include <lauxlib.h>
#include <lualib.h>



typedef struct mqttc_client_lua {
    lua_State *L;
    int on_conack_ref;
    int on_publish_ref;
    int on_message_ref;
}MQTT_CLINT_LUA_T;


typedef struct mqtt_topic_lua {
    char *topic;
    int topic_len;
    char *payload;
    int payload_len;
    MQTT_CLINT_LUA_T *mqtt_client;
}MQTT_TOPIC_LUA_T;


typedef struct mqtt_topic{
    const char *topic;
    uint8_t qos;
}MQTT_TOPIC_T;

#endif /* LUAMQTT */


#endif /* LUAMQTTC */
