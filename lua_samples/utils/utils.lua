-- utils module

str = "hello, this is tuyaopen"

-- base64
encodeStr = utils.base64_encode(str)
decodeStr = utils.base64_decode(encodeStr)

-- md5
utils.md5(str)

-- crc8,16,32
utils.crc8(str)
utils.crc16(str)
utils.crc32(str)

-- sha1,sha256
utils.sha1(str)
utils.sha256(str)

-- hmac
key = "1qaz3edc"
utils.hmac_md5(str, key)
utils.hmac_sha1(str, key)
utils.hmac_sha256(str, key)

