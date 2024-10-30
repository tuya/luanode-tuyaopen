------------------------------------------------------------------------------
-- Custom HTTPS Implementation using lua-mbedtls
-- Copyright (C) 2024 Tuya Inc.
------------------------------------------------------------------------------

local socket = require("socket")
local tls = require("tls")  -- 使用你的 tls.lua 模块
local ltn12 = require("ltn12")
local http = require("socket.http")
local url = require("socket.url")

local try = socket.try

--
-- Module
--
local _M = {
  _VERSION = "0.6",
  _COPYRIGHT = "Custom HTTPS Implementation - Copyright (C) 2024 Tuya Inc.",
  PORT = 443,
}

-- TLS configuration
local cfg = {
  protocol = "any",
  options = {"all", "no_sslv2", "no_sslv3"},
  verify = "none",
}

--------------------------------------------------------------------
-- Auxiliary Functions
--------------------------------------------------------------------

-- Insert default HTTPS port.
local function default_https_port(u)
   return url.build(url.parse(u, {port = _M.PORT}))
end

-- Convert an URL to a table according to Luasocket needs.
local function urlstring_totable(url, body, result_table)
   url = {
      url = default_https_port(url),
      method = body and "POST" or "GET",
      sink = ltn12.sink.table(result_table)
   }
   if body then
      url.source = ltn12.source.string(body)
      url.headers = {
         ["content-length"] = #body,
         ["content-type"] = "application/x-www-form-urlencoded",
      }
   end
   return url
end

-- Return a function which performs the SSL/TLS connection.
local function tcp(params)
    params = params or {}
    -- Default settings
    for k, v in pairs(cfg) do
       params[k] = params[k] or v
    end
    -- Force client mode
    params.mode = "tls-client"  -- 明确设置为客户端模式
    -- 'create' function for LuaSocket
    return function ()
        local conn = {}
        conn.sock = try(socket.tcp())
        local st = getmetatable(conn.sock).__index.settimeout
        function conn:settimeout(...)
            return st(self.sock, ...)
        end
        -- Replace TCP's connection function
        function conn:connect(host, port)
            self.sock:settimeout(socket.http.TIMEOUT)
            try(self.sock:connect(host, port))

            -- Wrap the socket with TLS
            local ssl_sock, msg = tls.wrap(self.sock, params)  -- 使用 tls.wrap 包装 socket
            if not ssl_sock then return nil, msg end

            conn.sock = ssl_sock  -- 更新连接为 SSL socket
            conn.sock:settimeout(socket.http.TIMEOUT)
            return 1
        end
        return conn
    end
end

--------------------------------------------------------------------
-- Main Function
--------------------------------------------------------------------

-- Make an HTTP request over secure connection. This function receives
--  the same parameters of LuaSocket's HTTP module (except 'proxy' and
--  'redirect') plus TLS parameters.
--
-- @param url mandatory (string or table)
-- @param body optional (string)
-- @return (string if url == string or 1), code, headers, status
--
local function request(url, body)
  local result_table = {}
  local stringrequest = type(url) == "string"
  if stringrequest then
    url = urlstring_totable(url, body, result_table)
  else
    url.url = default_https_port(url.url)
  end

  if http.PROXY or url.proxy then
    return nil, "proxy not supported; try https_proxy instead"
  elseif url.redirect then
    return nil, "redirect not supported"
  elseif url.create then
    return nil, "create function not permitted"
  end

  -- New 'create' function to establish a secure connection
  url.create = tcp(url)
  local res, code, headers, status = http.request(url)
  if res and stringrequest then
    return table.concat(result_table), code, headers, status
  end
  return res, code, headers, status
end

--------------------------------------------------------------------------------
-- Export module
--

_M.request = request

return _M
