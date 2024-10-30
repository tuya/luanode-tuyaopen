#ifndef F9A96612_BB1A_42B1_A48C_2A2F96B40080
#define F9A96612_BB1A_42B1_A48C_2A2F96B40080

#ifndef LUAT_MSGBUS_H
#define LUAT_MSGBUS_H

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "tuya_cloud_types.h"
#include "lua.h"

#define MSG_TIMER 1

typedef int (*LUA_MSG_HANDLE) (lua_State *L, void* ptr);

typedef struct rtos_msg{
    LUA_MSG_HANDLE handler;
    void* ptr;
    int arg1;
    int arg2;
}TUYA_RTOS_MSG_T;


// 定义接口方法
void lua_msgbus_init(void);
OPERATE_RET lua_msgbus_put(TUYA_RTOS_MSG_T* msg, size_t timeout);
OPERATE_RET lua_msgbus_get(TUYA_RTOS_MSG_T* msg, size_t timeout);
#define lua_msgbug_put2(ABC1,ABC2,ABC3,ABC4,ABC5) {\
    TUYA_RTOS_MSG_T _msg = {.handler=ABC1,.ptr=ABC2,.arg1=ABC3,.arg2=ABC4};\
    lua_msgbus_put(&_msg, ABC5);\
}

#endif


#endif /* F9A96612_BB1A_42B1_A48C_2A2F96B40080 */
