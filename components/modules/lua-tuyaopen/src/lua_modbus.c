#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "tuya_iot_config.h"

#if defined(LUA_LIB_MODBUS) && (LUA_LIB_MODBUS == 1)
#include "tuya_modbus_master.h"
#include "tkl_gpio.h"
STATIC int rs485_waittime = 10;
STATIC TUYA_GPIO_NUM_E rs485_dir_pin = 0;


STATIC INT_T rs_485_rts(BOOL_T on)
{
    if (on) {
        tkl_gpio_write(rs485_dir_pin,TUYA_GPIO_LEVEL_HIGH);
    }
    else {
        tkl_system_sleep(rs485_waittime);
        tkl_gpio_write(rs485_dir_pin,TUYA_GPIO_LEVEL_LOW);
    }
    return 0;
}

static int modbus_rtu_master_init(lua_State *L)
{
    int ret = 0;
    int portid = luaL_checkinteger(L,1);
    int baud = luaL_checkinteger(L,2);
    int rs485 = lua_isnoneornil(L, 3) ? FALSE : luaL_checkinteger(L, 3);
    if (rs485) {
        rs485_dir_pin = luaL_checkinteger(L,4);
        rs485_waittime = lua_isnoneornil(L, 5) ? 20 : luaL_checkinteger(L, 5);
        ret = tuya_modbus_rtu_master_init(portid,baud,rs485,rs485_dir_pin,rs_485_rts);
    }
    else
        ret = tuya_modbus_rtu_master_init(portid,baud,rs485,rs485_dir_pin,NULL);

    lua_pushinteger(L,ret);
    return 1;
}
static int modbus_rtu_set_salve_addr(lua_State *L)
{
    OPERATE_RET ret = 0;

    int  slave_addr = luaL_checkinteger(L,1);
    ret = tuya_modbus_rtu_set_salve_addr(slave_addr);
    if (ret != TY_MBS_OK) {
        lua_writestringerror("tuya_modbus_rtu_set_salve_addr failed %d",ret);
    }
    lua_pushinteger(L,ret);
    return 1;
}

// local value = modbus.read_discrete_registers(reg,2)
// if value ~= nil then
//     rtos.log("value:",value)
// end
static int modbus_rtu_master_read_discrete_registers(lua_State *L)
{
    OPERATE_RET ret = 0;
    UINT16_T value[32] = {0};
    int  reg_addr = luaL_checkinteger(L,1);
    int  reg_count = luaL_checkinteger(L,2) < 32 ? luaL_checkinteger(L,2):32;

    ret = tuya_modbus_rtu_master_read_discrete_registers(reg_addr,reg_count,value);
    if (ret == OPRT_OK) {
        lua_newtable(L);
        for (int i = 0; i < reg_count; i ++) {
            lua_pushinteger(L, i + 1); // Lua table index starts from 1
            lua_pushinteger(L, value[i]);
            lua_settable(L, -3);
        }
    }
    else{
        lua_writestringerror("tuya_modbus_rtu_master_read_discrete_registers failed %d",ret);
        return 0;
    }
    return 1;
}

static int modbus_rtu_master_read_hold_registers(lua_State *L)
{
    OPERATE_RET ret = 0;
    UINT16_T value[32] = {0};

    int  reg_addr = luaL_checkinteger(L,1);
    int  reg_count = luaL_checkinteger(L,2) < 32 ? luaL_checkinteger(L,2):32;


    ret = tuya_modbus_rtu_master_read_hold_registers(reg_addr,reg_count,value);
    if (ret == OPRT_OK) {
        lua_newtable(L);
        for (int i = 0; i < reg_count; i ++) {
            lua_pushinteger(L, i + 1); // Lua table index starts from 1
            lua_pushinteger(L, value[i]);
            lua_settable(L, -3);
        }
    }
    else{
        lua_writestringerror("tuya_modbus_rtu_master_read_hold_registers failed %d",ret);
        return 0;
    }
    return 1;
}

static int modbus_rtu_master_get_excetion_code(lua_State *L)
{
    lua_pushinteger(L,tuya_modbus_rtu_master_get_excetion_code());
    return 1;
}

static int modbus_rtu_master_error_recovery(lua_State *L)
{
    lua_pushinteger(L,tuya_modbus_rtu_master_error_recovery());
    return 1;
}

static int modbus_rtu_master_read_hold_coils(lua_State *L)
{
    OPERATE_RET ret = 0;
    BYTE_T value[32] = {0};
    int  reg_addr = luaL_checkinteger(L,1);
    int  reg_count = luaL_checkinteger(L,2) < 32 ? luaL_checkinteger(L,2):32;
    ret = tuya_modbus_rtu_master_read_hold_coils(reg_addr,reg_count,value);
    if (ret == OPRT_OK) {
        lua_newtable(L);
        for (int i = 0; i < reg_count; i ++) {
            lua_pushinteger(L, i + 1); // Lua table index starts from 1
            lua_pushinteger(L, value[i]);
            lua_settable(L, -3);
        }
    }
    else{
        lua_writestringerror("tuya_modbus_rtu_master_read_hold_coils failed %d",ret);
        return 0;
    }
    return 1;
}

static int modbus_rtu_master_read_discrete_coils(lua_State *L)
{
    OPERATE_RET ret = 0;
    BYTE_T value[32] = {0};
    int  reg_addr = luaL_checkinteger(L,1);
    int  reg_count = luaL_checkinteger(L,2) < 32 ? luaL_checkinteger(L,2):32;

    ret = tuya_modbus_rtu_master_read_discrete_coils(reg_addr,reg_count,value);
    if (ret == OPRT_OK) {
        lua_newtable(L);
        for (int i = 0; i < reg_count; i ++) {
            lua_pushinteger(L, i + 1); // Lua table index starts from 1
            lua_pushinteger(L, value[i]);
            lua_settable(L, -3);
        }
    }
    else{
        lua_writestringerror("tuya_modbus_rtu_master_read_discrete_coils failed %d",ret);
        return 0;
    }
    return 1;
}

static int modbus_rtu_master_write_single_coils(lua_State *L)
{
    OPERATE_RET ret = 0;

    int  reg_addr = luaL_checkinteger(L,1);
    int  value = luaL_checkinteger(L,2);

    ret = tuya_modbus_rtu_master_write_single_coils(reg_addr,value);
    if (ret != OPRT_OK) {
        lua_writestringerror("tuya_modbus_rtu_master_read_discrete_coils failed %d",ret);
    }
    lua_pushinteger(L,ret);
    return 1;
}

static int modbus_rtu_master_write_single_register(lua_State *L)
{
    OPERATE_RET ret = 0;

    int  reg_addr = luaL_checkinteger(L,1);
    int  value = luaL_checkinteger(L,2);

    ret = tuya_modbus_rtu_master_write_single_register(reg_addr,value);
    if (ret != OPRT_OK) {
        lua_writestringerror("tuya_modbus_rtu_master_write_single_register failed %d",ret);
    }
    lua_pushinteger(L,ret);
    return 1;
}
/*
    modbus.master_write_muilt_coils(0x55,{0x00,0x01})
 */
static int modbus_rtu_master_write_muilt_coils(lua_State *L)
{
    OPERATE_RET ret = 0;
    int num = 0;
    BYTE_T value[32] = {0};
    int  reg_addr = luaL_checkinteger(L,1);
    luaL_checktype(L, 2, LUA_TTABLE); // Check if the first argument is a table
    num = luaL_len(L, 2) > 32 ?32:luaL_len(L, 2); // Get the length of the table
    for (int i = 1; i <= num; i++) {
        lua_rawgeti(L, 2, i); // Get the value at index i in the table
        value[i-1] = (BYTE_T)lua_tointeger(L, -1); // Convert the value to an integer and add it to the sum
        lua_pop(L, 1); // Remove the value from the stack
    }
    ret = tuya_modbus_rtu_master_write_muilt_coils(reg_addr,num,value);
    if (ret != OPRT_OK) {
        lua_writestringerror("tuya_modbus_rtu_master_write_muilt_coils failed %d",ret);
    }
    lua_pushinteger(L,ret);
    return 1;
}

static int modbus_rtu_master_write_muilt_register(lua_State *L)
{
    OPERATE_RET ret = 0;
    int num = 0;
    UINT16_T value[32] = {0};
    int  reg_addr = luaL_checkinteger(L,1);
    luaL_checktype(L, 2, LUA_TTABLE); // Check if the first argument is a table
    num = luaL_len(L, 2) > 32 ?32:luaL_len(L, 2); // Get the length of the table
    for (int i = 1; i <= num; i++) {
        lua_rawgeti(L, 2, i); // Get the value at index i in the table
        value[i-1] = lua_tointeger(L, -1); // Convert the value to an integer and add it to the sum
        lua_pop(L, 1); // Remove the value from the stack
    }
    ret = tuya_modbus_rtu_master_write_muilt_register(reg_addr,num,value);
    if (ret != OPRT_OK) {
        lua_writestringerror("tuya_modbus_rtu_master_write_muilt_register failed %d",ret);
    }
    lua_pushinteger(L,ret);
    return 1;
}


#include "rotable2.h"
static const rotable_Reg_t rtu_modbus_master_lib[] = {
    {"init",                        ROREG_FUNC(modbus_rtu_master_init)},
    {"set_salve",                   ROREG_FUNC(modbus_rtu_set_salve_addr)},
    {"read_discrete_registers",     ROREG_FUNC(modbus_rtu_master_read_discrete_registers)},
    {"read_hold_registers",         ROREG_FUNC(modbus_rtu_master_read_hold_registers)},
    {"get_excetion_code",           ROREG_FUNC(modbus_rtu_master_get_excetion_code)},
    {"error_recovery",              ROREG_FUNC(modbus_rtu_master_error_recovery)},
    {"read_hold_coils",             ROREG_FUNC(modbus_rtu_master_read_hold_coils)},
    {"read_discrete_coils",         ROREG_FUNC(modbus_rtu_master_read_discrete_coils)},
    {"write_single_coils",          ROREG_FUNC(modbus_rtu_master_write_single_coils)},
    {"write_single_register",       ROREG_FUNC(modbus_rtu_master_write_single_register)},
    {"write_muilt_coils",           ROREG_FUNC(modbus_rtu_master_write_muilt_coils)},
    {"write_muilt_register",        ROREG_FUNC(modbus_rtu_master_write_muilt_register)},
    {NULL, ROREG_INT(0)}
};

LUAMOD_API int luaopen_rtu_modbus_master(lua_State *L)
{
    luaL_newlib2(L, rtu_modbus_master_lib);
    return 1;
}
#endif