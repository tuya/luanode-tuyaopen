/*
** $Id: linit.c,v 1.39.1.1 2017/04/19 17:20:42 roberto Exp $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB
#include "tuya_iot_config.h"
/*
** If you embed Lua in your program and need to open the standard
** libraries, call luaL_openlibs in your program. If you need a
** different set of libraries, copy this file to your project and edit
** it to suit your needs.
**
** You can also *preload* libraries, so that a later 'require' can
** open the library, which is already linked to the application.
** For that, do the following code:
**
**  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
**  lua_pushcfunction(L, luaopen_modname);
**  lua_setfield(L, -2, modname);
**  lua_pop(L, 1);  // remove PRELOAD table
*/

#define LUA_LIB_ZBUFF
#define LUA_LIB_CJSON
#include "lprefix.h"
#include <stddef.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#if defined(ENABLE_UART)
LUAMOD_API int luaopen_uartlib(lua_State *L);
#endif

#if defined(ENABLE_GPUI)
LUAMOD_API int luaopen_gpio_lib(lua_State *L);
#endif

// LUAMOD_API int luaopen_event(lua_State *L);
LUAMOD_API int luaopen_tuyaos(lua_State *L);
// LUAMOD_API int luaopen_kv(lua_State *L);
LUAMOD_API int luaopen_connlib(lua_State *L);
LUAMOD_API int luaopen_timer(lua_State *L);
LUAMOD_API int luaopen_socket_core(lua_State *L);
LUALIB_API int luaopen_luaproc( lua_State *L );
LUAMOD_API int luaopen_system (lua_State *L);
LUAMOD_API int luaopen_crypto( lua_State *L );
LUAMOD_API int luaopen_mqttc(lua_State *L);

#if defined(LUA_LIB_ZBUFF)
LUAMOD_API int luaopen_zbuff(lua_State *L);
#endif

#if defined(LUA_LIB_CJSON)
LUAMOD_API int luaopen_cjson(lua_State *L);
#endif

#if defined(LUA_LIB_MODBUS)
LUAMOD_API int luaopen_rtu_modbus_master(lua_State *L);
#endif


// LUAMOD_API int luaopen_ringbuff(lua_State *L);
// LUAMOD_API int luaopen_cffi(lua_State *L);
// LUAMOD_API int luaopen_alien_c(lua_State  *L);
/*
** these libs are loaded by lua.c and are readily available to any Lua
** program
*/
static const luaL_Reg loadedlibs[] = {
  {"_G", luaopen_base},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_STRLIBNAME, luaopen_string},
  
#if defined(LUA_COMPAT_BITLIB)
  {LUA_BITLIBNAME, luaopen_bit32},
#endif
#if defined(LUA_LIB_MATH)
  {LUA_MATHLIBNAME, luaopen_math},
#endif
#if defined(ENABLE_UART)
  {"uart",luaopen_uartlib},
#endif
#if defined(ENABLE_GPIO)
  {"gpio",luaopen_gpio_lib},
#endif
  // {"event",luaopen_event},
  {"tuyaos",luaopen_tuyaos},
  {"conn",luaopen_connlib},
  {"timer",luaopen_timer},
  {"luaproc",luaopen_luaproc},  
  {"sys",luaopen_system},
  {"utils",luaopen_crypto},
  {"mqtt", luaopen_mqttc},
  // {"kv",luaopen_kv},
#if defined(LUA_LIB_MODBUS)
  {"modbus_m", luaopen_rtu_modbus_master},
#endif
#if defined(LUA_LIB_CJSON)
  {"json",luaopen_cjson},
#endif
#if defined(LUA_LIB_ZBUFF)
  {"zbuff",luaopen_zbuff},
#endif
  // {"ringbuff",luaopen_ringbuff},
#ifdef ENABLE_ALIEN
  {"alien_c",luaopen_alien_c},
#endif
#ifdef ENABLE_FFI
  {"ffi",luaopen_cffi},
#endif

  {NULL, NULL}
};


LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }

  luaopen_socket_core(L);
  lua_setglobal(L, "socket");
}

