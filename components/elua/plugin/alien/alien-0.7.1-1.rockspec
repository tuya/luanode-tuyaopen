package="alien"
version="0.7.1-1"
source = {
  url = "git://github.com/mascarenhas/alien.git",
  branch = "v0.7.1"
}
description = {
  summary = "Lua->C FFI",
  detailed = [[
    Alien lets a Lua application call load dynamic libraries and call C functions
    in a portable way, using libffi.
  ]],
  license = "MIT/X11",
  homepage = "http://mascarenhas.github.com/alien"
}
dependencies = {
  "lua >= 5.1"
}
external_dependencies = {
  FFI = { library = "ffi" }
}
build = {
  type = "command",
  build_command = "./configure LUA=$(LUA) CPPFLAGS='-I$(LUA_INCDIR) -I$(FFI_INCDIR)' LDFLAGS=-L$(FFI_LIBDIR) --prefix=$(PREFIX) --libdir=$(LIBDIR) --datadir=$(LUADIR) && make clean && make",
  install_command = "make install",
  copy_directories = {}
}
