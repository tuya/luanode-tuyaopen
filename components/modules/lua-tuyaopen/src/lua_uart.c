#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "tkl_uart.h"
#include "tkl_memory.h"
#include "lua_msgbus.h"
#include "lua_zbuff.h"

#define MAX_UART_PORTS 4
typedef struct {
    int cb_ref;
} CallbackData;

static CallbackData tx_cb_data[MAX_UART_PORTS] = {{LUA_NOREF}};

static CallbackData rx_cb_data[MAX_UART_PORTS] = {{LUA_NOREF}};
/*
配置串口参数
@api    uart.setup(id, baud_rate, data_bits, stop_bits, partiy, flowctrl)
@int 串口id, uart0写0, uart1写1, 如此类推, 最大值取决于设备
@int 波特率, 默认115200，可选择波特率表:{2000000,921600,460800,230400,115200,57600,38400,19200,9600,4800,2400}
@int 数据位，默认为8, 可选 7/8
@int 停止位，默认为1, 根据实际情况，可以有0.5/1/1.5/2等
@int 校验位，可选 uart.None/uart.Even/uart.Odd
@int 流控，  可选 uart.None/RTSCTS/uart.XONXOFF/uart.DTRDSR
@return int 成功返回0,失败返回其他值
@usage
-- 最常用115200 8N1
uart.setup(1, 115200, 8, 1, uart.NONE)
*/
static int l_uart_setup(lua_State *L)
{
    TUYA_UART_NUM_E port_id = luaL_checkinteger(L, 1);
    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }

    TUYA_UART_BASE_CFG_T uart_config = {
        .baudrate = luaL_optinteger(L, 2, 115200),
        .databits = luaL_optinteger(L, 3, TUYA_UART_DATA_LEN_8BIT),
        .parity = luaL_optinteger(L, 4, TUYA_UART_PARITY_TYPE_NONE),
        .stopbits = luaL_optinteger(L, 5, TUYA_UART_STOP_LEN_1BIT),
        .flowctrl = luaL_optinteger(L, 6, TUYA_UART_FLOWCTRL_NONE),
    };

    int result = tkl_uart_init(port_id,&uart_config);
    lua_pushinteger(L, result);

    return 1;
}
static int l_uart_close(lua_State *L)
{
    TUYA_UART_NUM_E port_id = luaL_checkinteger(L, 1);
    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }
    int result = tkl_uart_deinit(port_id);
    if (result == OPRT_OK) {
        if (tx_cb_data[port_id].cb_ref) {
            luaL_unref(L,LUA_REGISTRYINDEX,tx_cb_data[port_id].cb_ref);
            tx_cb_data[port_id].cb_ref = LUA_NOREF;
        }
        if (rx_cb_data[port_id].cb_ref) {
            luaL_unref(L,LUA_REGISTRYINDEX,rx_cb_data[port_id].cb_ref);
            rx_cb_data[port_id].cb_ref =LUA_NOREF;
        }
    }
    lua_pushinteger(L, result);
    return 1;
}
// Helper function to read TUYA_UART_BASE_CFG_T from a Lua table
static TUYA_UART_BASE_CFG_T read_tuya_uart_base_cfg(lua_State *L, int index) {
    TUYA_UART_BASE_CFG_T cfg;

    lua_getfield(L, index, "baudrate");
    cfg.baudrate = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "parity");
    cfg.parity = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "databits");
    cfg.databits = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "stopbits");
    cfg.stopbits = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "flowctrl");
    cfg.flowctrl = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    return cfg;
}
static int l_uart_ioctl(lua_State *L)
{
    TUYA_UART_BASE_CFG_T cfg;
    int result ;
    TUYA_UART_NUM_E port_id = luaL_checkinteger(L, 1);
    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }
    lua_Number cmd = luaL_checkinteger(L, 2);

    if (cmd == TUYA_UART_RECONFIG_CMD) {
        cfg = read_tuya_uart_base_cfg(L, 3);
        result = tkl_uart_ioctl(port_id,cmd,(void*)&cfg);
    }
    else {
        void* arg = lua_touserdata(L,3);
        result = tkl_uart_ioctl(port_id,cmd,arg);
    }

    lua_pushinteger(L, result);
    return 1;
}

/*
写串口
@api    uart.write(id, data)
@int 串口id, uart0写0, uart1写1
@string/zbuff 待写入的数据，如果是zbuff会从指针起始位置开始读
@int 可选，要发送的数据长度，默认全发
@return int 成功的数据长度
@usage
-- 写入可见字符串
uart.write(1, "rdy\r\n")
-- 写入十六进制的数据串
uart.write(1, string.char(0x55,0xAA,0x4B,0x03,0x86))
-- 写入1024字节的数据
local hex_data = ""
for i = 1, 1024 do
    hex_data = hex_data .. string.char((i - 1) % 256)
end
uart.write(1, hex_data)
*/
static int l_uart_write(lua_State *L)
{
    const char *buf = NULL;
    int port_id = luaL_checkinteger(L, 1);
    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }
    size_t len;
    if(lua_isuserdata(L, 2)){
        LUA_ZBUFF_T *buff = ((LUA_ZBUFF_T *)luaL_checkudata(L, 2, LUA_ZBUFF_TYPE));
        len = buff->len - buff->cursor;
        buf = (const char *)(buff->addr + buff->cursor);
    }
    else {
        buf = luaL_checklstring(L, 2, &len);
    }

    uint16_t data_len = (uint16_t)len;

    int ret = tkl_uart_write((TUYA_UART_NUM_E)port_id, (void *)buf, data_len);

    lua_pushinteger(L, ret);
    return 1;
}
/*
读串口
@api    uart.read(id, len)
@int 串口id, uart0写0, uart1写1
@int 读取长度
@return string 读取到的数据
@usage
uart.read(1, 16)
*/
static int l_uart_read(lua_State *L)
{
    int port_id = luaL_checkinteger(L, 1);
    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }
    uint16_t len = (uint16_t)luaL_optinteger(L, 2, 1024);

    if(lua_isuserdata(L, 3)) {
        LUA_ZBUFF_T *buff = ((LUA_ZBUFF_T *)luaL_checkudata(L, 3, LUA_ZBUFF_TYPE));
        uint8_t* recv = buff->addr+buff->cursor;
        if(len > buff->len - buff->cursor)
            len = buff->len - buff->cursor;
        int result = tkl_uart_read((unsigned int)port_id, (void *)recv, len);
        if(result < 0)
            result = 0;
        buff->cursor += result;
        lua_pushinteger(L, result);
        return 1;
    }
    if (len < 128)
        len = 128;
    char *buff = (char *)tkl_system_malloc(len);
    if (buff == NULL) {
        luaL_error(L, "Failed to allocate memory for buffer");
    }

    int ret = tkl_uart_read((unsigned int)port_id, (void *)buff, len);

    if (ret > 0) {
        lua_pushlstring(L, buff, ret);
    } else {
        lua_pushnil(L);
    }

    tkl_system_free(buff);
    return 1;
}


/*
等待串口的数据量
@api    uart.waitRxSize(id,timeout)
@int 串口id, uart0写0, uart1写1
@int 超时timeout 单位毫秒
@return int 返回读到的长度
@usage
local size = uart.rxSize(1,100)
*/
static int l_uart_wait_rx_size(lua_State *L)
{
    uint8_t id = luaL_checkinteger(L, 1);
    uint16_t timeout = luaL_optinteger(L, 2, 0);
    int ret = tkl_uart_wait_for_data(id,timeout);
    lua_pushinteger(L, ret);
    return 1;
}

int l_tx_uart_handler(lua_State *L, void* ptr)
{
    (void)ptr; // unused

    TUYA_RTOS_MSG_T* msg = (TUYA_RTOS_MSG_T*)lua_topointer(L, -1);
    int port_id = msg->arg1;
    if (tx_cb_data[port_id].cb_ref == LUA_NOREF)
        return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, tx_cb_data[port_id].cb_ref);
    if (!lua_isnil(L, -1)) {
        lua_pushinteger(L, msg->arg1);
        lua_call(L, 1, 0);
    }
    return 0;
}
static void l_tkl_uart_tx_irq_cb(TUYA_UART_NUM_E port_id) {
    if (port_id >= MAX_UART_PORTS || tx_cb_data[port_id].cb_ref == LUA_NOREF) {
        return;
    }
    TUYA_RTOS_MSG_T msg = {0};

    msg.handler = l_tx_uart_handler;
    msg.ptr = NULL;
    msg.arg1 = port_id;
    msg.arg2= 0;
    lua_msgbus_put(&msg, 0);
    return;
}


static int l_uart_tx_irq_cb_reg(lua_State *L) {
    int port_id = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }

    lua_pushvalue(L, 2);
    tx_cb_data[port_id].cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    tkl_uart_tx_irq_cb_reg((TUYA_UART_NUM_E)port_id, l_tkl_uart_tx_irq_cb);

    return 1;
}


int l_rx_uart_handler(lua_State *L, void* ptr)
{
    (void)ptr; // unused
    TUYA_RTOS_MSG_T* msg = (TUYA_RTOS_MSG_T*)lua_topointer(L, -1);
    int port_id = msg->arg1;

    if (rx_cb_data[port_id].cb_ref == LUA_NOREF)
        return 0;

    lua_rawgeti(L, LUA_REGISTRYINDEX, rx_cb_data[port_id].cb_ref);
    if (!lua_isnil(L, -1)) {
        lua_pushinteger(L, msg->arg1);
        lua_call(L, 1, 0);
    }
    return 0;
}

static void l_tkl_uart_rx_irq_cb(TUYA_UART_NUM_E port_id) {

    if (port_id >= MAX_UART_PORTS || rx_cb_data[port_id].cb_ref == LUA_NOREF) {
        return;
    }
    TUYA_RTOS_MSG_T msg = {0};

    msg.handler = l_rx_uart_handler;
    msg.ptr = NULL;
    msg.arg1 = port_id;
    msg.arg2= 0;
    lua_msgbus_put(&msg, 0);
}
static int l_uart_rx_irq_cb_reg(lua_State *L)
{
    int port_id = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    if (port_id >= MAX_UART_PORTS) {
        luaL_error(L, "Invalid port_id");
    }

    lua_pushvalue(L, 2);
    rx_cb_data[port_id].cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    tkl_uart_rx_irq_cb_reg((TUYA_UART_NUM_E)port_id, l_tkl_uart_rx_irq_cb);
    return 0;
}

#include "rotable2.h"
static const rotable_Reg_t uart[] = {
    {"open",            ROREG_FUNC(l_uart_setup)},
    {"write",           ROREG_FUNC(l_uart_write)},
    {"read",            ROREG_FUNC(l_uart_read)},
    {"close",           ROREG_FUNC(l_uart_close)},
    {"set_tx_irq",      ROREG_FUNC(l_uart_tx_irq_cb_reg)},
    {"set_rx_irq",      ROREG_FUNC(l_uart_rx_irq_cb_reg)},
    {"wait_rx",         ROREG_FUNC(l_uart_wait_rx_size)},
    {"ioctl",           ROREG_FUNC(l_uart_ioctl)},
    {"UART_0",          ROREG_INT(TUYA_UART_NUM_0)},
    {"UART_1",          ROREG_INT(TUYA_UART_NUM_1)},
    {"UART_2",          ROREG_INT(TUYA_UART_NUM_2)},
    {"DATA_5_BITS",     ROREG_INT(TUYA_UART_DATA_LEN_5BIT)},
    {"DATA_6_BITS",     ROREG_INT(TUYA_UART_DATA_LEN_6BIT)},
    {"DATA_7_BITS",     ROREG_INT(TUYA_UART_DATA_LEN_7BIT)},
    {"DATA_8_BITS",     ROREG_INT(TUYA_UART_DATA_LEN_8BIT)},
    {"PARITY_NONE",     ROREG_INT(TUYA_UART_PARITY_TYPE_NONE)},
    {"PARITY_ODD",      ROREG_INT(TUYA_UART_PARITY_TYPE_ODD)},
    {"PARITY_EVEN",     ROREG_INT(TUYA_UART_PARITY_TYPE_EVEN)},
    {"STOPBITS_1",      ROREG_INT(TUYA_UART_STOP_LEN_1BIT)},
    {"STOPBITS_1_5",    ROREG_INT(TUYA_UART_STOP_LEN_1_5BIT1)},
    {"STOPBITS_2",      ROREG_INT(TUYA_UART_STOP_LEN_2BIT)},
    {"FLOWCTRL_NONE",   ROREG_INT(TUYA_UART_FLOWCTRL_NONE)},
    {"FLOWCTRL_RTSCTS", ROREG_INT(TUYA_UART_FLOWCTRL_RTSCTS)},
    {"FLOWCTRL_XONXOFF",ROREG_INT(TUYA_UART_FLOWCTRL_XONXOFF)},
    {"FLOWCTRL_DTRDSR", ROREG_INT(TUYA_UART_FLOWCTRL_DTRDSR)},
    {"SUSPEND",         ROREG_INT(TUYA_UART_SUSPEND_CMD)},
    {"FLUSH",           ROREG_INT(TUYA_UART_FLUSH_CMD)},
    {"RECONFIG",        ROREG_INT(TUYA_UART_RECONFIG_CMD)},
    {NULL, ROREG_INT(0)}
};

LUAMOD_API int luaopen_uartlib(lua_State *L)
{
    luaL_newlib2(L, uart);
    // Register the TUYA_UART_IOCTL_CMD_E constants

    return 1;
}

/*
local uart = require("uart")


local port_id = 0
local cmd = uart.TUYA_UART_RECONFIG_CMD
local arg = {
    baudrate = 115200,
    parity = 0, -- Replace with the appropriate value
    databits = 8, -- Replace with the appropriate value
    stopbits = 1, -- Replace with the appropriate value
    flowctrl = 0 -- Replace with the appropriate value
}

local result = uart.ioctl(port_id, cmd, arg)

if result == 0 then
    print("Operation successful")
else
    print("Operation failed with error code:", result)
end




 */