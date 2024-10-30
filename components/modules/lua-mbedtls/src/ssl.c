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
#include  "tal_system.h"
#include "mbedtls/version.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/base64.h"
#include "common.h"
#include "defcert.h"

#if MBEDTLS_VERSION_MAJOR < 3
#define MBEDTLS_PRIVATE(name) name
#define mbedtls_pk_parse_keyfile(ctx, path, pwd, f_rng, p_rng) mbedtls_pk_parse_keyfile(ctx, path, pwd)
#define mbedtls_pk_parse_key(ctx, key, keylen, pwd, pwdlen, f_rng, p_rng) mbedtls_pk_parse_key(ctx, key, keylen, pwd, pwdlen)
#define mbedtls_pk_check_pair(pub, prv, f_rng, p_rng) mbedtls_pk_check_pair(pub, prv)
#endif

#define TYPE_SSL_BASE "mbedtls.ssl.base"
#define TYPE_SSL_CONFIG "mbedtls.ssl.config"
#define TYPE_SSL_CONTEXT "mbedtls.ssl.context"

typedef struct {
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context drbg;
} Base;

typedef struct {
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert, cert;
	mbedtls_pk_context pkey;
	int mode;
    const unsigned char *psk;  // PSK
    size_t psk_len;            // PSK 长度
    const char *psk_identity;  // PSK 身份
    size_t psk_identity_len;   // PSK 身份长度
} Config;

typedef struct {
	mbedtls_ssl_context ssl;
	Config *cfg;
	int res;
	lua_State *L;
	void *buf;
	size_t len;
	uint32_t ms1, ms2;
	uint64_t ut;
} Context;

static char BASE;

static int freebase(lua_State *L) {
	Base *base = luaL_checkudata(L, 1, TYPE_SSL_BASE);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	mbedtls_entropy_free(&base->entropy);
	mbedtls_ctr_drbg_free(&base->drbg);
	return 0;
}

static Base *getbase(lua_State *L) {
	Base *base;
	lua_rawgetp(L, LUA_REGISTRYINDEX, &BASE);
	base = lua_touserdata(L, -1);
	lua_pop(L, 1);
	if (base) return base;
	base = lua_newuserdata(L, sizeof *base);
	if (luaL_newmetatable(L, TYPE_SSL_BASE)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushcfunction(L, freebase);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
	mbedtls_entropy_init(&base->entropy);
	mbedtls_ctr_drbg_init(&base->drbg);

	checkresult(L, mbedtls_ctr_drbg_seed(&base->drbg, mbedtls_entropy_func, &base->entropy, 0, 0));
	lua_rawsetp(L, LUA_REGISTRYINDEX, &BASE);
	return base;
}

static const char *const modes[] = {"tls-client", "tls-server", "dtls-client", "dtls-server", 0};

static int freeconfig(lua_State *L) {
	Config *cfg = luaL_checkudata(L, 1, TYPE_SSL_CONFIG);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	mbedtls_ssl_config_free(&cfg->conf);
	mbedtls_x509_crt_free(&cfg->cacert);
	mbedtls_x509_crt_free(&cfg->cert);
	mbedtls_pk_free(&cfg->pkey);
	return 0;
}

extern int __tuya_tls_random(void *p_rng, unsigned char *output, size_t output_len);
/* ARG: mode, [cacert], [cert],[pkey],[psk_identity],[psk]
** RES: cfg */
static int f_newconfig(lua_State *L) {
	int mode = luaL_checkoption(L, 1, 0, modes);
	const char *cacert = luaL_optstring(L, 2, 0);
	const char *cert = luaL_optstring(L, 3, 0);
    const char *pkey = luaL_optstring(L, 4, NULL);
    const char *psk_identity = luaL_optstring(L, 5, NULL); // 从 Lua 获取 PSK 身份
    const char *psk_hex = luaL_optstring(L, 6, NULL); // 从 Lua 获取 PSK (16进制字符串)
    size_t psk_len = 0;

	Base *base = getbase(L);
	Config *cfg = lua_newuserdata(L, sizeof *cfg);
	cfg->mode = mode;

    // 解析 PSK (如果提供)
    if (psk_hex) {
        psk_len = strlen(psk_hex) / 2; // 计算 PSK 长度
        cfg->psk = malloc(psk_len);
        for (size_t i = 0; i < psk_len; i++) {
            sscanf(psk_hex + (i * 2), "%2hhx", &cfg->psk[i]);
        }
    }
    cfg->psk_identity = psk_identity;
    cfg->psk_identity_len = psk_identity ? strlen(psk_identity) : 0;


	if (luaL_newmetatable(L, TYPE_SSL_CONFIG)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushcfunction(L, freeconfig);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
	mbedtls_ssl_config_init(&cfg->conf);
	mbedtls_x509_crt_init(&cfg->cacert);
	mbedtls_x509_crt_init(&cfg->cert);
	mbedtls_pk_init(&cfg->pkey);
	mbedtls_ssl_config_defaults(&cfg->conf, !!(mode & 1), !!(mode & 2), 0);
	mbedtls_ssl_conf_rng(&cfg->conf, mbedtls_ctr_drbg_random, &base->drbg);
	mbedtls_ssl_conf_authmode(&cfg->conf, MBEDTLS_SSL_VERIFY_NONE);
   // 配置 PSK
    if (cfg->psk && cfg->psk_identity) {
        checkresult(L, mbedtls_ssl_conf_psk(&cfg->conf, cfg->psk, psk_len, cfg->psk_identity, cfg->psk_identity_len));
    }

	if (cacert) {

        char *tmp_cacert = tal_malloc(strlen(cacert)+1);
        memcpy(tmp_cacert, cacert, strlen(cacert));

		if (mbedtls_x509_crt_parse(&cfg->cacert, tmp_cacert,strlen(cacert)+1)) {
            tal_free(tmp_cacert);
            return luaL_error(L, "%s: can't parse CA certificate", cacert);
        }
        tal_free(tmp_cacert);
		mbedtls_ssl_conf_ca_chain(&cfg->conf, &cfg->cacert, 0);
		mbedtls_ssl_conf_authmode(&cfg->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
	}
	if (cert) {

        char *tmp_cert = tal_malloc(strlen(cert)+1);
        strcpy(tmp_cert, cert);
        tmp_cert[strlen(tmp_cert)] = '\0';
		if (mbedtls_x509_crt_parse(&cfg->cert, tmp_cert,strlen(tmp_cert)+1)) {
            return luaL_error(L, "%s: can't parse certificate", cert);
        }
        if (mbedtls_pk_parse_key(&cfg->pkey,(const unsigned char*)pkey,strlen(pkey), NULL, 0, __tuya_tls_random, NULL)) {
            return luaL_error(L, "%s: can't parse client private key", cert);
        }

		if (mbedtls_pk_check_pair(&cfg->cert.pk, &cfg->pkey, __tuya_tls_random, &base->drbg)) {
            return luaL_error(L, "%s: certificate/private key mismatch", cert);
        }
		checkresult(L, mbedtls_ssl_conf_own_cert(&cfg->conf, &cfg->cert, &cfg->pkey));
	}
    else if (mode & 1) { /* Use default certificate in server mode */
		checkresult(L, mbedtls_x509_crt_parse(&cfg->cert, defcert, sizeof defcert));
		checkresult(L, mbedtls_pk_parse_key(&cfg->pkey, defpkey, sizeof defpkey, 0, 0, __tuya_tls_random, &base->drbg));
		checkresult(L, mbedtls_ssl_conf_own_cert(&cfg->conf, &cfg->cert, &cfg->pkey));
	}
	return 1;
}

static Context *checkcontext(lua_State *L, int arg) {
	Context *ctx = luaL_checkudata(L, arg, TYPE_SSL_CONTEXT);
	luaL_argcheck(L, !ctx->res, arg, "context is busy");
	return ctx;
}

static void pincontext(lua_State *L, Context *ctx, int idx) {
	lua_pushvalue(L, idx);
	lua_rawsetp(L, LUA_REGISTRYINDEX, ctx);
	ctx->L = L;
}

static int unpincontext(lua_State *L, Context *ctx, int res) {
	char msg[256];
	lua_pushnil(L);
	lua_rawsetp(L, LUA_REGISTRYINDEX, ctx);
	ctx->res = 0;
	if (res >= 0) return 0; /* Success */
	lua_pushnil(L);
	if (res == -1) { /* Callback error */
		lua_insert(L, -2);
		return -1;
	}
	if (res == MBEDTLS_ERR_SSL_BAD_INPUT_DATA) { /* Misconfiguration */
		lua_pushliteral(L, "invalid operation");
		return -1;
	}
	if (res == MBEDTLS_ERR_SSL_WANT_READ) {
		lua_pushliteral(L, "want-read");
		return -1;
	}
	if (res == MBEDTLS_ERR_SSL_WANT_WRITE) {
		lua_pushliteral(L, "want-write");
		return -1;
	}
	if (res == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
		lua_pushliteral(L, "close-notify");
		return -1;
	}
	checkresult(L, mbedtls_ssl_session_reset(&ctx->ssl));
	mbedtls_strerror(res, msg, sizeof msg);
	lua_pushfstring(L, "mbedtls error %d (%s)", res, msg);
	return -1;
}

/* RES: timeout */
static int m_gettimeout(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	if (ctx->ms2) lua_pushnumber(L, ctx->ms2 / 1000.0);
	else lua_pushnil(L);
	return 1;
}

/* ARG: rcb, wcb, [ref] */
static int m_setbio(lua_State *L) {
	checkcontext(L, 1);
	checknonil(L, 2);
	checknonil(L, 3);
	lua_settop(L, 4);
#if LUA_VERSION_NUM < 504
	lua_getuservalue(L, 1);
	lua_insert(L, 2);
	lua_rawseti(L, 2, 1);
	lua_rawseti(L, 2, 2);
	lua_rawseti(L, 2, 3);
#else
	lua_setiuservalue(L, 1, 1);
	lua_setiuservalue(L, 1, 2);
	lua_setiuservalue(L, 1, 3);
#endif
	return 0;
}

/* ARG: peerid */
static int m_setpeerid(lua_State *L) {
#if defined(MBEDTLS_SSL_DTLS_HELLO_VERIFY)
	Context *ctx = checkcontext(L, 1);
	size_t len;
	const unsigned char *buf = checkdata(L, 2, &len);
	checkopsup(L, ctx->cfg->mode == 3, 1); /* DTLS server only */
	checkresult(L, mbedtls_ssl_set_client_transport_id(&ctx->ssl, buf, len));
#else
    return luaL_error(L, "Setting peer ID is not supported in this build (MBEDTLS_SSL_DTLS_HELLO_VERIFY not defined).");
#endif
	return 0;
}

/* ARG: hostname */
static int m_sethostname(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	const char *hostname = luaL_optstring(L, 2, 0);
	luaL_checkany(L, 2);
	checkopsup(L, !(ctx->cfg->mode & 1), 1); /* Client only */
	checkresult(L, mbedtls_ssl_set_hostname(&ctx->ssl, hostname));
	return 0;
}

/* RES: true | nil, error */
static int m_handshake(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	int res = 0;
	pincontext(L, ctx, 1);
	while (ctx->ssl.MBEDTLS_PRIVATE(state) != MBEDTLS_SSL_HANDSHAKE_OVER) {
		if ((res = mbedtls_ssl_handshake_step(&ctx->ssl))) break;
		if (ctx->ssl.MBEDTLS_PRIVATE(state) == MBEDTLS_SSL_SERVER_HELLO_DONE && ctx->cfg->mode == 3) break; /* DTLS server only */
	}
	if (unpincontext(L, ctx, res)) return 2;
	lua_pushboolean(L, 1);
	return 1;
}

/* ARG: size
** RES: data | nil, error */
static int m_read(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	lua_Integer size = luaL_checkinteger(L, 2);
	unsigned char buf[MBEDTLS_SSL_IN_CONTENT_LEN];
	int res;
	checkrange(L, size >= 0, 2);
	pincontext(L, ctx, 1);
	if (size > MBEDTLS_SSL_IN_CONTENT_LEN) size = MBEDTLS_SSL_IN_CONTENT_LEN;
	if (unpincontext(L, ctx, res = mbedtls_ssl_read(&ctx->ssl, buf, size))) return 2;
	lua_pushlstring(L, (char *)buf, res);
	return 1;
}

/* ARG: data, [pos], [len]
** RES: size | nil, error */
static int m_write(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	size_t size;
	const unsigned char *data = checkdata(L, 2, &size);
	size_t pos = luaL_optinteger(L, 3, 1) - 1;
	size_t len = luaL_optinteger(L, 4, size - pos);
	int res;
	checkrange(L, pos <= size, 3);
	checkrange(L, len <= size - pos, 4);
	pincontext(L, ctx, 1);
	if (unpincontext(L, ctx, res = mbedtls_ssl_write(&ctx->ssl, data + pos, len))) return 2;
	lua_pushinteger(L, res);
	return 1;
}

/* RES: true | nil, error */
static int m_closenotify(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	pincontext(L, ctx, 1);
	if (unpincontext(L, ctx, mbedtls_ssl_close_notify(&ctx->ssl))) return 2;
	lua_pushboolean(L, 1);
	return 1;
}

static int m_reset(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	checkresult(L, mbedtls_ssl_session_reset(&ctx->ssl));
	return 0;
}

static int m__gc(lua_State *L) {
	Context *ctx = checkcontext(L, 1);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
    if (ctx->cfg && ctx->cfg->psk) {
        tal_free(ctx->cfg->psk);
    }
	mbedtls_ssl_free(&ctx->ssl);
	return 0;
}

static const luaL_Reg t_context[] = {
	{"gettimeout", m_gettimeout},
	{"setbio", m_setbio},
	{"setpeerid", m_setpeerid},
	{"sethostname", m_sethostname},
	{"handshake", m_handshake},
	{"read", m_read},
	{"write", m_write},
	{"closenotify", m_closenotify},
	{"reset", m_reset},
	{"__gc", m__gc},
	{0, 0}
};

static int pushcb(lua_State *L, void *p, int n) {
	lua_rawgetp(L, LUA_REGISTRYINDEX, p);
#if LUA_VERSION_NUM < 504
	lua_getuservalue(L, -1);
	lua_rawgeti(L, -1, n);
	lua_rawgeti(L, -2, 1);
#else
	lua_getiuservalue(L, -1, n);
	lua_getiuservalue(L, -2, 1);
#endif
	if (!lua_isnil(L, -1)) return 1;
	lua_pop(L, 1);
	return 0;
}

static int isinteger(lua_State *L, int idx, lua_Integer *val) {
	lua_Integer i;
#if LUA_VERSION_NUM < 503
	lua_Number n;
	if (!lua_isnumber(L, idx)) return 0;
	n = lua_tonumber(L, idx);
	i = (lua_Integer)n;
	if (i != n) return 0;
#else
	int res;
	i = lua_tointegerx(L, idx, &res);
	if (!res) return 0;
#endif
	*val = i;
	return 1;
}

static int readf(lua_State *L) {
	Context *ctx = lua_touserdata(L, 1);
	const char *buf;
	size_t len;
	int narg = pushcb(L, ctx, 3);
	lua_pushinteger(L, ctx->len);
	lua_call(L, narg + 1, 1); /* Call 'rcb([ref,] size)' */
	if (!(buf = lua_tolstring(L, -1, &len))) goto error;
	if (!len) {
		ctx->res = MBEDTLS_ERR_SSL_WANT_READ;
		return 0;
	}
	if (len > ctx->len) goto error;
	memcpy(ctx->buf, buf, len);
	ctx->res = len;
	return 0;
error:
	return luaL_error(L, "invalid read result");
}

static int writef(lua_State *L) {
	Context *ctx = lua_touserdata(L, 1);
	lua_Integer len;
	int narg = pushcb(L, ctx, 2);
	lua_pushlstring(L, ctx->buf, ctx->len);
	lua_call(L, narg + 1, 1); /* Call 'wcb([ref,] data)' */
	if (!isinteger(L, -1, &len)) goto error;
	if (!len) {
		ctx->res = MBEDTLS_ERR_SSL_WANT_WRITE;
		return 0;
	}
	if (len < 0 || len > (lua_Integer)ctx->len) goto error;
	ctx->res = len;
	return 0;
error:
	return luaL_error(L, "invalid write result");
}

static int read_cb(void *p, unsigned char *buf, size_t len) {
	Context *ctx = p;
	ctx->res = -1;
	ctx->buf = buf;
	ctx->len = len;
	lua_cpcall(ctx->L, readf, ctx);
	return ctx->res;
}

static int write_cb(void *p, const unsigned char *buf, size_t len) {
	Context *ctx = p;
	ctx->res = -1;
	ctx->buf = (void *)buf;
	ctx->len = len;
	lua_cpcall(ctx->L, writef, ctx);
	return ctx->res;
}

static uint64_t getutime(void) {
    return (uint64_t)tal_system_get_millisecond();
}

static void settimer_cb(void *p, uint32_t ms1, uint32_t ms2) {
	Context *ctx = p;
	ctx->ms1 = ms1;
	ctx->ms2 = ms2;
	if (!ms2) return;
	ctx->ut = getutime();
}

static int gettimer_cb(void *p) {
	Context *ctx = p;
	uint32_t ms;
	if (!ctx->ms2) return -1;
	ms = (getutime() - ctx->ut) / 1000; /* Elapse on overflow */
	if (ms >= ctx->ms2) return 2;
	if (ms >= ctx->ms1) return 1;
	return 0;
}

/* ARG: cfg, rcb, wcb, [ref]
** RES: ctx */
static int f_newcontext(lua_State *L) {
	Config *cfg = luaL_checkudata(L, 1, TYPE_SSL_CONFIG);
	Context *ctx;
	checknonil(L, 2);
	checknonil(L, 3);
	lua_settop(L, 4);
#if LUA_VERSION_NUM < 504
	ctx = lua_newuserdata(L, sizeof *ctx);
#else
	ctx = lua_newuserdatauv(L, sizeof *ctx, 4);
#endif
	ctx->cfg = cfg;
	ctx->res = 0;
	lua_insert(L, 1);
#if LUA_VERSION_NUM < 504
	lua_createtable(L, 4, 0);
	lua_insert(L, 2);
	lua_rawseti(L, 2, 1);
	lua_rawseti(L, 2, 2);
	lua_rawseti(L, 2, 3);
	lua_rawseti(L, 2, 4);
	lua_setuservalue(L, 1);
#else
	lua_setiuservalue(L, 1, 1);
	lua_setiuservalue(L, 1, 2);
	lua_setiuservalue(L, 1, 3);
	lua_setiuservalue(L, 1, 4);
#endif
	if (luaL_newmetatable(L, TYPE_SSL_CONTEXT)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
#if LUA_VERSION_NUM < 502
		luaL_register(L, 0, t_context);
#else
		luaL_setfuncs(L, t_context, 0);
#endif
	}
	lua_setmetatable(L, -2);
	mbedtls_ssl_init(&ctx->ssl);
	mbedtls_ssl_set_bio(&ctx->ssl, ctx, write_cb, read_cb, 0);
	mbedtls_ssl_set_timer_cb(&ctx->ssl, ctx, settimer_cb, gettimer_cb);
	checkresult(L, mbedtls_ssl_setup(&ctx->ssl, &cfg->conf));
	return 1;
}

static const luaL_Reg l_ssl[] = {
	{"newconfig", f_newconfig},
	{"newcontext", f_newcontext},
	{0, 0}
};

int luaopen_mbedtls_ssl(lua_State *L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, "mbedtls.ssl", l_ssl);
#else
	luaL_newlib(L, l_ssl);
#endif
	return 1;
}
