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

#include <mbedtls/md.h>
#include "common.h"

static char hexchar(int x) {
	return x >= 10 ? x + 'a' - 10 : x + '0';
}

static void pushresult(lua_State *L, int idx, const unsigned char *hash, size_t len) {
	if (lua_toboolean(L, idx)) lua_pushlstring(L, (const char *)hash, len);
	else {
		char buf[MBEDTLS_MD_MAX_SIZE * 2];
		size_t pos = 0;
		len <<= 1;
		while (pos < len) {
			int x = *hash++;
			buf[pos++] = hexchar(x >> 4);
			buf[pos++] = hexchar(x & 0xf);
		}
		lua_pushlstring(L, buf, len);
	}
}

/* ARG: type, data, [raw]
** RES: hash */
static int f_hash(lua_State *L) {
	const mbedtls_md_info_t *info = mbedtls_md_info_from_string(luaL_checkstring(L, 1));
	size_t size;
	const unsigned char *data = checkdata(L, 2, &size);
	unsigned char hash[MBEDTLS_MD_MAX_SIZE];
	checkvalue(L, info, 1);
	checkresult(L, mbedtls_md(info, data, size, hash));
	pushresult(L, 3, hash, mbedtls_md_get_size(info));
	return 1;
}

/* ARG: type, key, data, [raw]
** RES: hash */
static int f_hmac(lua_State *L) {
	const mbedtls_md_info_t *info = mbedtls_md_info_from_string(luaL_checkstring(L, 1));
	size_t klen, size;
	const unsigned char *key = checkdata(L, 2, &klen);
	const unsigned char *data = checkdata(L, 3, &size);
	unsigned char hash[MBEDTLS_MD_MAX_SIZE];
	checkvalue(L, info, 1);
	checkresult(L, mbedtls_md_hmac(info, key, klen, data, size, hash));
	pushresult(L, 4, hash, mbedtls_md_get_size(info));
	return 1;
}

static const luaL_Reg l_md[] = {
	{"hash", f_hash},
	{"hmac", f_hmac},
	{0, 0}
};

int luaopen_mbedtls_md(lua_State *L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, "mbedtls.md", l_md);
#else
	luaL_newlib(L, l_md);
#endif
	return 1;
}
