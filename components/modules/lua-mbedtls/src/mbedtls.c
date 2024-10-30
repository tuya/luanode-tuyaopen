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

#include <string.h>
#include "common.h"

static const luaL_Reg funcs[] = {
	{0, 0}
};

static const luaL_Reg libs[] = {
	{"mbedtls.base64", luaopen_mbedtls_base64},
	{"mbedtls.md", luaopen_mbedtls_md},
	{"mbedtls.ssl", luaopen_mbedtls_ssl},
	{0, 0}
};

int luaopen_mbedtls(lua_State *L) {
	const luaL_Reg *lib;
#if LUA_VERSION_NUM < 502
	luaL_register(L, "mbedtls", funcs);
#else
	luaL_newlib(L, funcs);
#endif
	for (lib = libs; lib->name; ++lib) {
		lua_pushstring(L, strchr(lib->name, '.') + 1);
#if LUA_VERSION_NUM < 502
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 1);
#else
		luaL_requiref(L, lib->name, lib->func, 0);
#endif
		lua_rawset(L, -3);
	}
	lua_pushliteral(L, MODNAME);
	lua_setfield(L, -2, "_NAME");
	lua_pushliteral(L, VERSION);
	lua_setfield(L, -2, "_VERSION");
	return 1;
}
