Mbed TLS module for Lua
=======================

[lua-mbedtls] adds support for [Mbed TLS] in Lua:
* SSL/TLS communication + cookie API.
* Message digest and HMAC.
* Base64 encoding/decoding.


Dependencies
------------

+ lua >= 5.1 (or luajit)
+ mbedtls >= 2.7


Building and installing with LuaRocks
-------------------------------------

To build and install, run:

    luarocks make

To install the latest release using [luarocks.org], run:

    luarocks install lua-mbedtls


Building and installing with CMake
----------------------------------

To build and install, run:

    cmake .
    make
    make install

To build for a specific Lua version, set `USE_LUA_VERSION`. For example:

    cmake -D USE_LUA_VERSION=5.1 .

or for LuaJIT:

    cmake -D USE_LUA_VERSION=jit .

To build in a separate directory, replace `.` with a path to the source.


Usage example: TLS over LuaSocket
---------------------------------

```Lua
local socket = require 'socket'
local ssl = require 'mbedtls.ssl'

local function read(h, n)
    return assert(h:receive(n))
end

local function write(h, s)
    return assert(h:send(s))
end

local tcp = assert(socket.connect('github.com', 443))
local cfg = ssl.newconfig('tls-client')
local ctx = ssl.newcontext(cfg, read, write, tcp)

ctx:write('GET / HTTP/1.0\r\n\r\n')
print(ctx:read(9999))

ctx:reset()
tcp:close()
```

Output:
```
HTTP/1.1 301 Moved Permanently
Content-Length: 0
Location: https://github.com/
connection: close
```

Please note that the above example is deliberately simplified for brevity.

Check out the [API Reference] for more information.


[lua-mbedtls]: https://github.com/neoxic/lua-mbedtls
[Mbed TLS]: https://www.trustedfirmware.org/projects/mbed-tls
[luarocks.org]: https://luarocks.org
[API Reference]: doc/
