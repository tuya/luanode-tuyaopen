------------------------------------------------------------------------------
-- Custom SSL Implementation using lua-mbedtls
-- Copyright (C) 2024 Tuya Inc.
------------------------------------------------------------------------------

local mbedtls = require("mbedtls")
local ssl = mbedtls.ssl

local unpack = table.unpack or unpack

-- We must prevent the contexts to be collected before the connections,
-- otherwise the C registry will be cleared.
local registry = setmetatable({}, {__mode="k"})

--
-- Auxiliary Functions
--

local function optexec(func, param, ctx)
  if param then
    if type(param) == "table" then
      return func(ctx, unpack(param))
    else
      return func(ctx, param)
    end
  end
  return true
end

-- Create a new SSL/TLS context based on the provided configuration
local function newcontext(cfg)
   local ctx, msg
   -- Create the context using mbedtls
   ctx, msg = ssl.newconfig(cfg.mode, cfg.cacert, cfg.cert, cfg.pkey, cfg.psk_identity, cfg.psk)
   if not ctx then return nil, msg end

   -- Set additional options if provided
   if cfg.verify then
      local success, errorMsg = optexec(ctx.setverify, cfg.verify, ctx)
      if not success then return nil, errorMsg end
   end

   if cfg.options then
      local success, errorMsg = optexec(ctx.setoptions, cfg.options, ctx)
      if not success then return nil, errorMsg end
   end

   if cfg.depth then
      local success, errorMsg = ctx.setdepth(cfg.depth)
      if not success then return nil, errorMsg end
   end

   return ctx
end

-- Wrap the socket with SSL/TLS configuration
local function wrap(sock, cfg)
   local ctx, msg
   if type(cfg) == "table" then
      ctx, msg = newcontext(cfg)  -- Create new context
      if not ctx then return nil, msg end
   else
      ctx = cfg
   end

   local ssl_sock, msg = ssl.newcontext(ctx, sock)  -- Use mbedtls to create SSL context
   if not ssl_sock then return nil, msg end

   sock:setfd(-1)  -- Mark original socket as invalid
   registry[ssl_sock] = ctx  -- Prevent context from being garbage collected
   return ssl_sock
end

-- Extract connection information
local function info(ssl, field)
  local info = ssl.info(ssl)  -- Get connection info
  if field then
    return info[field]
  end
  return info
end

-- Set method for SSL connections
ssl.setmethod("info", info)

--------------------------------------------------------------------------------
-- Export module
--

local _M = {
  _VERSION        = "0.6",
  _COPYRIGHT      = "Mbedtls Implementation - Copyright (C) 2020-2024 Tuya Inc.",
  newcontext      = newcontext,
  wrap            = wrap,
}

return _M
