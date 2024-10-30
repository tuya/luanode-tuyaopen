package = 'lua-mbedtls'
version = 'git-1'
source = {
	url = 'git://github.com/neoxic/lua-mbedtls.git',
}
description = {
	summary = 'Mbed TLS module for Lua',
	detailed = [[
		lua-mbedtls adds support for Mbed TLS in Lua:
		* SSL/TLS communication + cookie API.
		* Message digest and HMAC.
		* Base64 encoding/decoding.
	]],
	license = 'MIT',
	homepage = 'https://github.com/neoxic/lua-mbedtls',
	maintainer = 'Arseny Vakhrushev <arseny.vakhrushev@me.com>',
}
dependencies = {
	'lua >= 5.1',
}
external_dependencies = {
	MBEDTLS = {
		header = 'mbedtls/version.h',
		library = 'mbedtls',
	},
}
build = {
	type = 'builtin',
	modules = {
		mbedtls = {
			sources = {
				'src/base64.c',
				'src/main.c',
				'src/md.c',
				'src/ssl.c',
			},
			incdirs = '$(MBEDTLS_INCDIR)',
			libdirs = '$(MBEDTLS_LIBDIR)',
			libraries = {'mbedtls', 'mbedcrypto', 'mbedx509'},
		},
	},
}
