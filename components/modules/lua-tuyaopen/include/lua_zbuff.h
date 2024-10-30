#ifndef LUA_ZBUFF
#define LUA_ZBUFF
#ifndef LUAT_ZBUFF_H
#define LUAT_ZBUFF_H

#include "tuya_cloud_types.h"
#include "tal_memory.h"

#define LUA_ZBUFF_TYPE "ZBUFF*"
#define tozbuff(L) ((LUA_ZBUFF_T *)luaL_checkudata(L, 1, LUA_ZBUFF_TYPE))

#define ZBUFF_SEEK_SET 0
#define ZBUFF_SEEK_CUR 1
#define ZBUFF_SEEK_END 2

#if defined ( __CC_ARM )
#pragma anon_unions
#endif

typedef struct luat_zbuff {
    uint8_t* addr;      //数据存储的地址
    size_t len;       //实际分配空间的长度
    union {
    	size_t cursor;    //目前的指针位置，表明了处理了多少数据
    	size_t used;	//已经保存的数据量，表明了存了多少数据
    };

    uint32_t width; //宽度
    uint32_t height;//高度
    uint8_t bit;    //色深度
} LUA_ZBUFF_T;


int __zbuff_resize(LUA_ZBUFF_T *buff, uint32_t new_size);

#define lua_zbuff_malloc(a)     tal_malloc(a)
#define lua_zbuff_free(a)       tal_free(a)

// #define CONFIG_FRAMEBUFFER             1


#endif


#endif /* LUA_ZBUFF */
