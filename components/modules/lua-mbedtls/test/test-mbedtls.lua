local base64 = require 'mbedtls.base64'
local md = require 'mbedtls.md'
local ssl = require 'mbedtls.ssl'

assert(base64.encode('1234') == 'MTIzNA==')
assert(base64.encode('~~~1234~~~', 4, 4) == 'MTIzNA==')
assert(base64.decode('MTIzNA==') == '1234')
assert(base64.decode('~~~MTIzNA==~~~', 4, 8) == '1234')

assert(not pcall(base64.encode, '1234', 2, 4))
assert(not pcall(base64.decode, 'MTIzNA==', 2, 8))
assert(not pcall(base64.decode, 'MTI*NA==')) -- Invalid character

assert(md.hash('MD5', 'The quick brown fox jumps over the lazy dog') == '9e107d9d372bb6826bd81d3542a419d6')
assert(md.hmac('MD5', 'key', 'The quick brown fox jumps over the lazy dog') == '80070713463e7749b90c2dc24911e275')
assert(md.hmac('SHA1', 'key', 'The quick brown fox jumps over the lazy dog') == 'de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9')
assert(md.hmac('SHA256', 'key', 'The quick brown fox jumps over the lazy dog') == 'f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8')

local peerid = 'abc'
local cookie = ssl.getcookie(peerid)
assert(ssl.checkcookie(peerid, cookie))

local ref = {}
local s1 = ''
local s2 = ''

local function read1(r, n)
	assert(r == ref)
	local d = s2:sub(1, n)
	s2 = s2:sub(#d + 1, #s2)
	return d
end

local function read2(n)
	local d = s1:sub(1, n)
	s1 = s1:sub(#d + 1, #s1)
	return d
end

local function write1(r, d)
	assert(r == ref)
	s1 = s1 .. d
	return #d
end

local function write2(d)
	s2 = s2 .. d
	return #d
end

---------
-- TLS --
---------

local cfg1 = ssl.newconfig('tls-client')
local cfg2 = ssl.newconfig('tls-server')

local ctx1 = ssl.newcontext(cfg1, read1, write1, ref)
local ctx2 = ssl.newcontext(cfg2, read2, write2)

assert(not pcall(ctx1.setpeerid, ctx1, 'abc'))
assert(not pcall(ctx2.setpeerid, ctx2, 'abc'))
assert(not pcall(ctx2.sethostname, ctx2, 'abc'))

local function testbio(ctx1, ctx2)
	assert(select(2, ctx1:read(10)) == 'want-read')
	assert(ctx1:write('abc') == 3)
	assert(ctx1:write('defg') == 4)
	assert(ctx1:closenotify())

	assert(ctx2:read(10) == 'abc')
	assert(ctx2:read(10) == 'defg')
	assert(select(2, ctx2:read(10)) == 'close-notify')

	ctx1:reset()
	ctx2:reset()

	ctx1:setbio(read1, write1, ref)
	ctx2:setbio(read2, write2)
end

for i = 1, 2 do
	ctx1:sethostname(nil)
	ctx1:sethostname('abc')
	repeat
		local ok1, err1 = ctx1:handshake()
		local ok2, err2 = ctx2:handshake()
		assert(ok1 or err1 == 'want-read')
		assert(ok2 or err2 == 'want-read')
	until ok1 and ok2
	testbio(ctx1, ctx2)
end

----------
-- DTLS --
----------

local cfg1 = ssl.newconfig('dtls-client')
local cfg2 = ssl.newconfig('dtls-server')

local ctx1 = ssl.newcontext(cfg1, read1, write1, ref)
local ctx2 = ssl.newcontext(cfg2, read2, write2)

assert(not pcall(ctx1.setpeerid, ctx1, 'abc'))
assert(not pcall(ctx2.sethostname, ctx2, 'abc'))

for i = 1, 1 do
	ctx1:sethostname(nil)
	ctx1:sethostname('abc')
	local verified
	repeat
		local ok1, err1 = ctx1:handshake()
		local ok2, err2 = ctx2:handshake()
		assert(ok1 or err1 == 'want-read')
		if verified then -- Second step
			assert(ok2 or err2 == 'want-read')
		elseif err2 ~= 'want-read' then
			if ok2 then -- First step
				verified = true
			else -- Peer will retry with 'ClientHello'
				ctx2:setpeerid('abc')
			end
		end
	until ok1 and ok2
	testbio(ctx1, ctx2)
end
