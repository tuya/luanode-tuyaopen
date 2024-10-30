#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "rotable2.h"
void luaL_newlib2(lua_State* l, const rotable_Reg_t* reg) {
  #ifdef LUAT_CONF_DISABLE_ROTABLE
  luaL_newlib(l,reg);
  int i;
  int nup = 0;

  luaL_checkstack(l, nup, "too many upvalues");
  for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(l, -nup);
        if (reg->value.value.func)
            lua_pushcclosure(l, reg->value.value.func, nup);  /* closure with those upvalues */
        else
            lua_pushinteger(l, reg->value.value.intvalue);
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);  /* remove upvalues */
  #else
  rotable2_newlib(l, reg);
  #endif
}