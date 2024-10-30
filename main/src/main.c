/**
 * @file main.c
 * @date 2024-07-19
 * 
 * @copyright Copyright (c) 2024 TuyaInc
 * 
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
// #include "lua.h"
// #include "luaconf.h"
#include "tal_api.h"
#include "uart_handler.h"
#include "lua_main.h"

static void create_lua_init_file(void)
{
	PR_INFO("Create init.lua");
	lfs_file_t file;
    int result = lfs_file_open(tal_lfs_get(), &file, "init.lua", LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
	if (LFS_ERR_OK != result) {
		PR_ERR("Failed to open file for writing");
		return;
	}
	lfs_file_write(tal_lfs_get(), &file, "print('Hello LuaNode'); print(_VERSION)\n", strlen("print('Hello LuaNode'); print(_VERSION)\n"));
	lfs_file_close(tal_lfs_get(), &file);
	PR_INFO("File written");
}

/**
 * init file system 
 */
static void lua_lfs_init(void)
{
	PR_INFO("lfs init");

	create_lua_init_file();
}

extern void tkl_log_output(const char *format, ...);
/**
 * @brief 
 * 
 */
void user_main(void)
{	
    tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);   
	tal_kv_init(&(tal_kv_cfg_t){.seed = "vmlkasdh93dlvlcy", .key = "dflfuap134ddlduq", });
	lua_lfs_init();
	uart_init();

	// lua startup
    char *lua_argv[] = {(char *)"lua", (char *)"-i", NULL};
    lua_main(2, lua_argv);
	

	while(1) {
		tal_system_sleep(1000);
	}
}

/**
 * @brief main
 *
 * @param argc
 * @param argv
 * @return void
 */
#if OPERATING_SYSTEM == SYSTEM_LINUX
void main(int argc, char *argv[])
{
    user_main();
}
#else

/* Tuya thread handle */
static THREAD_HANDLE ty_app_thread = NULL;

/**
 * @brief  task thread
 *
 * @param[in] arg:Parameters when creating a task
 * @return none
 */
static void tuya_app_thread(void *arg)
{
    user_main();

    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;
}

void tuya_app_main(void)
{
    THREAD_CFG_T thrd_param = {4096, 4, "tuya_app_main"};
    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
}
#endif
