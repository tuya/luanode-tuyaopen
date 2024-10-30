Base64
======

```Lua
local base64 = require 'mbedtls.base64'
```

### base64.encode(data, [pos], [len])
Encodes `data` into a Base64 representation. Optional `pos` marks the beginning of data (default is 1). Optional `len` marks the size of data (default is `#data - pos + 1`).

### base64.decode(data, [pos], [len])
Decodes Base64 `data`. Optional `pos` marks the beginning of data (default is 1). Optional `len` marks the size of data (default is `#data - pos + 1`).
