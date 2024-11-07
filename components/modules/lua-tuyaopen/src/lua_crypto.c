#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "crc.h"
#include "crc32i.h"
#include "tal_security.h"
#include "tal_memory.h"
#include "mbedtls/md5.h"

#define LUA_CRYPTO_TYPE "crypto"
/**
计算CRC16
@api crypto.crc16(method, data, poly, initial, finally, inReversem outReverse)
@string CRC16模式（"IBM","MAXIM","USB","MODBUS","CCITT","CCITT-FALSE","X25","XMODEM","DNP","USER-DEFINED"）
@string 字符串
@int poly值
@int initial值
@int finally值
@int 输入反转,1反转,默认0不反转
@int 输入反转,1反转,默认0不反转
@return int 对应的CRC16值
@usage
-- 计算CRC16
local crc = crypto.crc16("")
 */
static int l_crypto_crc16(lua_State *L)
{
    size_t inputlen;
    const unsigned char *inputData;
    const char  *inputmethod = (const char*)luaL_checkstring(L, 1);

    inputData = (const unsigned char*)lua_tolstring(L,2,&inputlen);

    uint16_t poly = luaL_optnumber(L,3,0x0000);
    uint16_t initial = luaL_optnumber(L,4,0x0000);
    uint16_t finally = luaL_optnumber(L,5,0x0000);
    uint8_t inReverse = luaL_optnumber(L,6,0);
    uint8_t outReverse = luaL_optnumber(L,7,0);
    lua_pushinteger(L, calcCRC16(inputData, inputmethod,inputlen,poly,initial,finally,inReverse,outReverse));
    return 1;
}


/**
直接计算modbus的crc16值
@api crypto.crc16_modbus(data)
@string 数据
@return int 对应的CRC16值
@usage
-- 计算CRC16 modbus
local crc = crypto.crc16_modbus(data)
 */
static int l_crypto_crc16_modbus(lua_State *L)
{
    size_t len = 0;
    const unsigned char *inputData = (const unsigned char*)luaL_checklstring(L, 1, &len);

    lua_pushinteger(L, calcCRC16_modbus(inputData, len));
    return 1;
}


/**
直接计算modbus的crc16值
@api crypto.crc16_modbus(data)
@string 数据
@return int 对应的CRC16值
@usage
-- 计算CRC16 modbus
local crc = crypto.crc16_modbus(data)
 */
static int l_crypto_crc16_array_modbus(lua_State *L)
{
    size_t len = 0;
    len = luaL_len(L, 1);                    // 获取数组长度
    unsigned char *pbuf = (unsigned char*)tal_malloc(len);        // 默认当做内存足够
    memset(pbuf, 0x00, len);
    for (int i = 1; i <= len; i++) {
        lua_rawgeti(L, 1, i);                       // 获取数组中的元素
        pbuf[i-1] = (unsigned char)lua_tonumber(L, -1);   // 转换为数字
        lua_pop(L, 1);                              // 从堆栈中移除该元素
    }
    lua_pushinteger(L, calcCRC16_modbus(pbuf, len));

    if (pbuf) {
        tal_free(pbuf);
        pbuf = NULL;
    }
    return 1;
}

/**
计算crc32值
@api crypto.crc32(data)
@string 数据
@return int 对应的CRC32值
@usage
-- 计算CRC32
local crc = crypto.crc32(data)
 */
static int l_crypto_crc32(lua_State *L)
{
    size_t len = 0;
    const unsigned char *inputData = (const unsigned char*)luaL_checklstring(L, 1, &len);

    lua_pushinteger(L, hash_crc32i_total((const void*)inputData, (unsigned int)len));
    return 1;
}

/**
计算crc8值
@api crypto.crc8(data, poly, start, revert)
@string 数据
@int crc多项式，可选，如果不写，将忽略除了数据外所有参数
@int crc初始值，可选，默认0
@boolean 是否需要逆序处理，默认否
@return int 对应的CRC8值
@usage
-- 计算CRC8
local crc = crypto.crc8(data)
local crc = crypto.crc8(data, 0x31, 0xff, false)
 */
static int l_crypto_crc8(lua_State *L)
{
    size_t len = 0;
    const unsigned char *inputData = (const unsigned char*)luaL_checklstring(L, 1, &len);
    if (!lua_isinteger(L, 2)) {
        lua_pushinteger(L, calcCRC8(inputData, len));
    } else {
    	uint8_t poly = lua_tointeger(L, 2);
    	uint8_t start = luaL_optinteger(L, 3, 0);
    	uint8_t is_rev = 0;
    	if (lua_isboolean(L, 4)) {
    		is_rev = lua_toboolean(L, 4);
    	}
    	uint8_t i;
    	uint8_t CRC8 = start;
		uint8_t *Src = (uint8_t *)inputData;
		if (is_rev)
		{
			poly = 0;
			for (i = 0; i < 8; i++)
			{
				if (start & (1 << (7 - i)))
				{
					poly |= 1 << i;
				}
			}
			while (len--)
			{

				CRC8 ^= *Src++;
				for (i = 0; i < 8; i++)
				{
					if ((CRC8 & 0x01))
					{
						CRC8 >>= 1;
						CRC8 ^= poly;
					}
					else
					{
						CRC8 >>= 1;
					}
				}
			}
		}
		else
		{
			while (len--)
			{

				CRC8 ^= *Src++;
				for (i = 8; i > 0; --i)
				{
					if ((CRC8 & 0x80))
					{
						CRC8 <<= 1;
						CRC8 ^= poly;
					}
					else
					{
						CRC8 <<= 1;
					}
				}
			}
		}
		lua_pushinteger(L, CRC8);
    }
    return 1;
}



static const unsigned char hexchars[] = "0123456789ABCDEF";
static void fixhex(const char* source, char* dst, size_t len) {
    for (size_t i = 0; i < len; i++)
    {
        char ch = *(source+i);
        dst[i*2] = hexchars[(unsigned char)ch >> 4];
        dst[i*2+1] = hexchars[(unsigned char)ch & 0xF];
    }
}
/**
计算md5值
@api crypto.md5(str)
@string 需要计算的字符串
@return string 计算得出的md5值的hex字符串
@usage
-- 计算字符串"abc"的md5
log.info("md5", crypto.md5("abc"))
 */
static int l_crypto_md5(lua_State *L) {
    size_t size = 0;
    const char* str = luaL_checklstring(L, 1, &size);
    char tmp[32] = {0};
    char dst[32] = {0};
    if (tal_md5_ret((const uint8_t*)str, size, (uint8_t*)tmp) == 0) {
        fixhex(tmp, dst, 16);
        lua_pushlstring(L, dst, 32);
        return 1;
    }
    return 0;
}

/**
计算hmac_md5值
@api crypto.hmac_md5(str, key)
@string 需要计算的字符串
@string 密钥
@return string 计算得出的hmac_md5值的hex字符串
@usage
-- 计算字符串"abc"的hmac_md5
log.info("hmac_md5", crypto.hmac_md5("abc", "1234567890"))
 */
static int l_crypto_hmac_md5(lua_State *L) {
    size_t str_size = 0;
    size_t key_size = 0;

    const char* str = luaL_checklstring(L, 1, &str_size);
    const char* key = luaL_checklstring(L, 2, &key_size);
    char tmp[32] = {0};
    char dst[32] = {0};

    mbedtls_md_context_t sha_ctx;
    int ret = 1;

    mbedtls_md_init(&sha_ctx);
    ret = mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if (0 != ret) {
        goto EXIT;
    }
    mbedtls_md_hmac_starts(&sha_ctx, (unsigned char *)key, key_size);
    mbedtls_md_hmac_update(&sha_ctx, (unsigned char *)str, str_size);
    mbedtls_md_hmac_finish(&sha_ctx, (unsigned char *)tmp);
EXIT:
    mbedtls_md_free(&sha_ctx);
    if (0 == ret) {
        fixhex(tmp, dst, 16);
        lua_pushlstring(L, dst, 32);
		return 1;
    }

    return 0;
}


/**
计算sha1值
@api crypto.sha1(str)
@string 需要计算的字符串
@return string 计算得出的sha1值的hex字符串
@usage
-- 计算字符串"abc"的sha1
log.info("sha1", crypto.sha1("abc"))
 */
static int l_crypto_sha1(lua_State *L) {
    size_t size = 0;
    const char* str = luaL_checklstring(L, 1, &size);
    char tmp[40] = {0};
    char dst[40] = {0};
    if (tal_sha1_ret((const uint8_t*)str, size, (uint8_t*)tmp) == 0) {
        fixhex(tmp, dst, 20);
        lua_pushlstring(L, (const char*)dst, 40);
        return 1;
    }
    return 0;
}


/**
计算hmac_sha1值
@api crypto.hmac_sha1(str, key)
@string 需要计算的字符串
@string 密钥
@return string 计算得出的hmac_sha1值的hex字符串
@usage
-- 计算字符串"abc"的hmac_sha1
log.info("hmac_sha1", crypto.hmac_sha1("abc", "1234567890"))
 */
static int l_crypto_hmac_sha1(lua_State *L) {
    size_t str_size = 0;
    size_t key_size = 0;
    const char* str = luaL_checklstring(L, 1, &str_size);
    const char* key = luaL_checklstring(L, 2, &key_size);
    char tmp[40] = {0};
    char dst[40] = {0};
    if (tal_sha1_mac((const uint8_t *)key, key_size, (const uint8_t *)str, str_size, (uint8_t*)tmp) == 0) {
        fixhex(tmp, dst, 20);
        lua_pushlstring(L, dst, 40);
        return 1;
    }
    return 0;
}

/**
计算sha256值
@api crypto.sha256(str)
@string 需要计算的字符串
@return string 计算得出的sha256值的hex字符串
@usage
-- 计算字符串"abc"的sha256
log.info("sha256", crypto.sha256("abc"))
 */
static int l_crypto_sha256(lua_State *L) {
    size_t size = 0;
    const char* str = luaL_checklstring(L, 1, &size);
    char tmp[64] = {0};
    char dst[64] = {0};
    if (tal_sha256_ret((const uint8_t*)str, size, (uint8_t*)tmp, 0) == 0) {
        fixhex(tmp, dst, 32);
        lua_pushlstring(L, dst, 64);
        return 1;
    }
    return 0;
}

typedef struct
{
    char tp[16];
    size_t key_len;
	mbedtls_md_context_t *ctx;
}lua_crypt_stream_t;

/*
创建流式hash用的stream
@api crypto.hash_init(tp)
@string hash类型, 大写字母, 例如 "MD5" "SHA1" "SHA256"
@string hmac值，可选
@return userdata 成功返回一个数据结构,否则返回nil
@usage
-- 无hmac的hash stream
local md5_stream = crypto.hash_init("MD5")
local sha1_stream = crypto.hash_init("SHA1")
local sha256_stream = crypto.hash_init("SHA256")

-- 带hmac的hash stream
local md5_stream = crypto.hash_init("MD5", "123456")
local sha1_stream = crypto.hash_init("SHA1", "123456")
local sha256_stream = crypto.hash_init("SHA256", "123456")
*/
static int l_crypt_hash_init(lua_State *L) {
    lua_crypt_stream_t *stream = (lua_crypt_stream_t *)lua_newuserdata(L, sizeof(lua_crypt_stream_t));
    if(stream == NULL) {
        lua_pushnil(L);
    }
	else {
        memset(stream, 0x00, sizeof(lua_crypt_stream_t));
        const char* key = NULL;
        const char* md = luaL_checkstring(L, 1);
        memcpy(stream->tp, md, strlen(md)+1);
        if(lua_type(L, 2) == LUA_TSTRING) {
            key = luaL_checklstring(L, 2, &(stream->key_len));
        }
		mbedtls_md_init(stream->ctx);
		int ret = mbedtls_md_setup(stream->ctx, mbedtls_md_info_from_string(md), 1);
		if (ret != 0) {
			mbedtls_md_free(stream->ctx);
			lua_pushnil(L);
			return 1;
		}
		if (stream->key_len > 0){
			mbedtls_md_hmac_starts(stream->ctx, (const unsigned char*)key, stream->key_len);
		}
		else {
			mbedtls_md_starts(stream->ctx);
		}
        luaL_setmetatable(L, LUA_CRYPTO_TYPE);
    }
    return 1;
}


/*
流式hash更新数据
@api crypto.hash_update(stream, data)
@userdata crypto.hash_init()创建的stream, 必选
@string 待计算的数据,必选
@return 无
@usage
crypto.hash_update(stream, "OK")
*/
static int l_crypt_hash_update(lua_State *L) {
    lua_crypt_stream_t *stream = (lua_crypt_stream_t *)luaL_checkudata(L, 1, LUA_CRYPTO_TYPE);
    size_t data_len = 0;
    const char *data = luaL_checklstring(L, 2, &data_len);
	mbedtls_md_hmac_update(stream->ctx, (unsigned char *)data, data_len);
    return 0;
}

/*
获取流式hash校验值并释放创建的stream
@api crypto.hash_finish(stream)
@userdata crypto.hash_init()创建的stream,必选
@return string 成功返回计算得出的流式hash值的hex字符串，失败无返回
@usage
local hashResult = crypto.hash_finish(stream)
*/
static int l_crypt_hash_finish(lua_State *L) {
    lua_crypt_stream_t *stream = (lua_crypt_stream_t *)luaL_checkudata(L, 1, LUA_CRYPTO_TYPE);
    char buff[128] = {0};
    char output[64];
	int ret = 0;
    if (stream->key_len > 0) {
        ret = mbedtls_md_hmac_finish(stream->ctx, (unsigned char*)output);
    }
    else {
        ret = mbedtls_md_finish(stream->ctx, (unsigned char*)output);
    }
	mbedtls_md_free(stream->ctx);
    if (ret != 0) {
        return 0;
    }
    fixhex(output, buff, ret);
    lua_pushlstring(L, buff, ret * 2);
    return 1;
}
#include "rotable2.h"
static const rotable_Reg_t reg_crypto[] =
{
    { "md5" ,           ROREG_FUNC(l_crypto_md5            )},
    { "sha1" ,          ROREG_FUNC(l_crypto_sha1           )},
    { "sha256" ,        ROREG_FUNC(l_crypto_sha256         )},
    { "hmac_md5" ,      ROREG_FUNC(l_crypto_hmac_md5       )},
    { "hmac_sha1" ,     ROREG_FUNC(l_crypto_hmac_sha1      )},
    { "crc16",          ROREG_FUNC(l_crypto_crc16          )},
    { "crc16_modbus",   ROREG_FUNC(l_crypto_crc16_modbus   )},
    { "crc16_modbus_array",ROREG_FUNC(l_crypto_crc16_array_modbus)},
    { "crc32",          ROREG_FUNC(l_crypto_crc32          )},
    { "crc8",           ROREG_FUNC(l_crypto_crc8           )},
    { "hash_init",      ROREG_FUNC(l_crypt_hash_init)},
    { "hash_update",    ROREG_FUNC(l_crypt_hash_update)},
    { "hash_finish",    ROREG_FUNC(l_crypt_hash_finish)},

	{ NULL,             ROREG_INT(0) }
};

LUAMOD_API int luaopen_crypto( lua_State *L ) {
    luaL_newlib2(L, reg_crypto);
    luaL_newmetatable(L, LUA_CRYPTO_TYPE);
    lua_pop(L, 1);
    return 1;
}
