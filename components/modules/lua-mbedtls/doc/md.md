Message Digest
==============

```Lua
local md = require 'mbedtls.md'
```

### md.hash(type, data, [raw])
Returns a message digest of `type` for `data` as a hexadecimal (or raw if `raw = true`) string.

### md.hmac(type, key, data, [raw])
Returns an HMAC of `type` for `key` and `data` as a hexadecimal (or raw if `raw = true`) string.

The `type` argument (a string) can be one of the following:
- `MD2`
- `MD4`
- `MD5`
- `SHA1`
- `SHA224`
- `SHA256`
- `SHA384`
- `SHA512`
- `RIPEMD160`
