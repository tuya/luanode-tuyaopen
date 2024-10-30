#ifndef B1196B17_B230_4F14_862F_045F143991BE
#define B1196B17_B230_4F14_862F_045F143991BE

int lua_task_start(char *luafile,int stack_size);

int lua_set_script_path(char *path,int index);

int lua_set_app_info(char *name,char *ver);

int lua_handle_input(bool force);

#endif /* B1196B17_B230_4F14_862F_045F143991BE */
