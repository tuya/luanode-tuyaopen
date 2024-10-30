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

#include <mbedtls/base64.h>
#include "common.h"

/* ARG: data, [pos], [len]
** RES: data */
static int f_encode(lua_State *L) {
	size_t size;
	const unsigned char *data = checkdata(L, 1, &size);
	size_t pos = luaL_optinteger(L, 2, 1) - 1;
	size_t len = luaL_optinteger(L, 3, size - pos);
	void *buf;
	checkrange(L, pos <= size, 2);
	checkrange(L, len <= size - pos, 3);
	mbedtls_base64_encode(0, 0, &size, data += pos, len);
	checkresult(L, mbedtls_base64_encode(buf = lua_newuserdata(L, size), size, &size, data, len));
	lua_pushlstring(L, buf, size);
	return 1;
}

/* ARG: data, [pos], [len]
** RES: data */
static int f_decode(lua_State *L) {
	size_t size;
	const unsigned char *data = checkdata(L, 1, &size);
	size_t pos = luaL_optinteger(L, 2, 1) - 1;
	size_t len = luaL_optinteger(L, 3, size - pos);
	void *buf;
	checkrange(L, pos <= size, 2);
	checkrange(L, len <= size - pos, 3);
	checkvalue(L, mbedtls_base64_decode(0, 0, &size, data += pos, len) != MBEDTLS_ERR_BASE64_INVALID_CHARACTER, 1);
	checkresult(L, mbedtls_base64_decode(buf = lua_newuserdata(L, size), size, &size, data, len));
	lua_pushlstring(L, buf, size);
	return 1;
}

static const luaL_Reg l_base64[] = {
	{"encode", f_encode},
	{"decode", f_decode},
	{0, 0}
};

int luaopen_mbedtls_base64(lua_State *L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, "mbedtls.base64", l_base64);
#else
	luaL_newlib(L, l_base64);
#endif
	return 1;
}
