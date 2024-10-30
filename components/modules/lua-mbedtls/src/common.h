/*
** Copyright (C) 2020-2022 Arseny Vakhrushev <arseny.vakhrushev@me.com>
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/

#pragma once
#include "tal_memory.h"
#include "lauxlib.h"
#include "mbedtls/error.h"

#define MODNAME "lua-mbedtls"
#define VERSION "0.2.2"

#define checkdata(L, arg, len) (const unsigned char *)luaL_checklstring(L, arg, len)
#define checknonil(L, arg) luaL_argcheck(L, !lua_isnoneornil(L, arg), arg, "value is nil")
#define checkvalue(L, cond, arg) luaL_argcheck(L, cond, arg, "invalid value")
#define checkrange(L, cond, arg) luaL_argcheck(L, cond, arg, "value out of range")
#define checkopsup(L, cond, arg) luaL_argcheck(L, cond, arg, "operation not supported")

#define checkresult(L, expr) { \
	int __err__ = (expr); \
	if (__err__) { \
		char __msg__[256]; \
		mbedtls_strerror(__err__, __msg__, sizeof __msg__); \
		luaL_error(L, "unexpected error %d (%s) at " __FILE__ ":%d", __err__, __msg__, __LINE__); \
	} \
}

#if LUA_VERSION_NUM < 502
#define lua_rawgetp(L, idx, key) (lua_pushlightuserdata(L, key), lua_rawget(L, idx))
#define lua_rawsetp(L, idx, key) (lua_pushlightuserdata(L, key), lua_insert(L, -2), lua_rawset(L, idx))
#define lua_getuservalue(L, idx) lua_getfenv(L, idx)
#define lua_setuservalue(L, idx) lua_setfenv(L, idx)
#elif !defined lua_cpcall
#define lua_cpcall(L, func, arg) (lua_pushcfunction(L, func), lua_pushlightuserdata(L, arg), lua_pcall(L, 1, 0, 0))
#endif

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int luaopen_mbedtls(lua_State *L);
EXPORT int luaopen_mbedtls_base64(lua_State *L);
EXPORT int luaopen_mbedtls_md(lua_State *L);
EXPORT int luaopen_mbedtls_ssl(lua_State *L);
