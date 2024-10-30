#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "tkl_gpio.h"

#include "lua_msgbus.h"


typedef struct {
    int lua_ref;
    int pin_id;
}LUA_GPIO_CTX_T;

// 保存中断回调的数组
static LUA_GPIO_CTX_T s_gpio_irq_ctx[TUYA_GPIO_NUM_MAX]={{LUA_NOREF,-1}};

static int l_tkl_gpio_init(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    int  direct = luaL_checkinteger(L,2);
    int mode = luaL_checkinteger(L,3);
    int level = luaL_checkinteger(L,4);
    TUYA_GPIO_BASE_CFG_T cfg={
        .direct = direct,
        .mode = mode,
        .level = level
    };
    int ret = tkl_gpio_init(pin,&cfg);

    lua_pushinteger(L,ret);
    return 1;
}

static int l_tkl_gpio_deinit(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    int ret = tkl_gpio_deinit(pin);
    if (ret == OPRT_OK && s_gpio_irq_ctx[pin].lua_ref) {
        luaL_unref(L,LUA_REGISTRYINDEX,s_gpio_irq_ctx[pin].lua_ref);
        s_gpio_irq_ctx[pin].lua_ref = LUA_NOREF;
    }
    lua_pushinteger(L,ret);
    return 1;
}

static int l_tkl_gpio_write(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    int level = luaL_checkinteger(L,2);
    int ret = tkl_gpio_write(pin,level);
    lua_pushinteger(L,ret);
    return 1;
}

static int l_tkl_gpio_read(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    TUYA_GPIO_LEVEL_E level;
    int ret = tkl_gpio_read(pin,&level);
    lua_pushinteger(L,ret);
    lua_pushinteger(L,level);
    return 2;
}

int l_gpio_handler(lua_State *L, void* ptr) {
    (void)ptr; // unused
    // 给 sys.publish方法发送数据
    TUYA_RTOS_MSG_T* msg = (TUYA_RTOS_MSG_T*)lua_topointer(L, -1);
    int pin = msg->arg1;
    if (pin < 0 || pin >= TUYA_GPIO_NUM_MAX)
        return 0;
    if (s_gpio_irq_ctx[pin].lua_ref == LUA_NOREF)
        return 0;
    lua_geti(L, LUA_REGISTRYINDEX, s_gpio_irq_ctx[pin].lua_ref);
    if (!lua_isnil(L, -1)) {
        lua_pushinteger(L, msg->arg2);
        lua_call(L, 1, 0);
    }
    return 0;
}

static void l_gpio_irq_default(void *args)
{
    TUYA_RTOS_MSG_T msg = {0};
    LUA_GPIO_CTX_T *gpio_ctx = (LUA_GPIO_CTX_T*)args;
    if (gpio_ctx) {
        msg.handler = l_gpio_handler;
        msg.ptr = NULL;
        msg.arg1 = gpio_ctx->pin_id;
        msg.arg2 = (int)args;
        lua_msgbus_put(&msg, 0);
    }
}
static int l_tkl_gpio_irq_init(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    int irq_mode = luaL_checkinteger(L,2);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    s_gpio_irq_ctx[pin].lua_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    TUYA_GPIO_IRQ_T cfg;
    cfg.mode = irq_mode;
    cfg.cb = l_gpio_irq_default;
    cfg.arg = &s_gpio_irq_ctx[pin];
    int ret = tkl_gpio_irq_init(pin,&cfg);
    lua_pushinteger(L,ret);
    return 1;
}

static int l_tkl_gpio_irq_enable(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    int ret = tkl_gpio_irq_enable(pin);
    lua_pushinteger(L,ret);
    return 1;
}

static int l_tkl_gpio_irq_disable(lua_State *L)
{
    int pin = luaL_checkinteger(L,1);
    int ret = tkl_gpio_irq_disable(pin);
    lua_pushinteger(L,ret);
    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t tkl_gpio_lib[]  = {
    {"init",        ROREG_FUNC(l_tkl_gpio_init)},
    {"deinit",      ROREG_FUNC(l_tkl_gpio_deinit)},
    {"read",        ROREG_FUNC(l_tkl_gpio_read)},
    {"write",       ROREG_FUNC(l_tkl_gpio_write)},
    {"set_irq",     ROREG_FUNC(l_tkl_gpio_irq_init)},
    {"irq_enable",  ROREG_FUNC(l_tkl_gpio_irq_enable)},
    {"irq_disable", ROREG_FUNC(l_tkl_gpio_irq_disable)},

    {"PULLUP" ,      ROREG_INT(TUYA_GPIO_PULLUP)},
    {"PULLDOWN",       ROREG_INT(TUYA_GPIO_PULLDOWN)},
    {"HIGH_IMPEDANCE",       ROREG_INT(TUYA_GPIO_HIGH_IMPEDANCE)},
    {"FLOATING",       ROREG_INT(TUYA_GPIO_FLOATING)},
    {"PUSH_PULL",       ROREG_INT(TUYA_GPIO_PUSH_PULL)},
    {"PENDRAIN_PULLUP",       ROREG_INT(TUYA_GPIO_OPENDRAIN_PULLUP)},
    {"HIGH_IMPEDANCE",       ROREG_INT(TUYA_GPIO_HIGH_IMPEDANCE)},
    {"IRQ_RISE",       ROREG_INT(TUYA_GPIO_IRQ_RISE)},
    {"IRQ_FALL",       ROREG_INT(TUYA_GPIO_IRQ_FALL)},
    {"IRQ_BOTH",       ROREG_INT(TUYA_GPIO_IRQ_RISE_FALL)},
    {"IRQ_LOW",       ROREG_INT(TUYA_GPIO_IRQ_LOW)},
    {"IRQ_HIGH",       ROREG_INT(TUYA_GPIO_IRQ_HIGH)},
    {"INPUT",       ROREG_INT(TUYA_GPIO_INPUT)},
    {"OUTPUT",       ROREG_INT(TUYA_GPIO_OUTPUT)},
    {"LOW",       ROREG_INT(TUYA_GPIO_LEVEL_LOW)},
    {"HIGH",       ROREG_INT(TUYA_GPIO_LEVEL_HIGH)},
    {NULL,	   ROREG_INT(0) }
};

LUAMOD_API int luaopen_gpio_lib(lua_State *L)
{
    luaL_newlib2(L, tkl_gpio_lib);
    return 1;
}