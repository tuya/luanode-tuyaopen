#include <stdio.h>
#include <string.h>
#include "uart_handler.h"
#include "lua.h"
#include "tal_api.h"

int uart_getc(char *c)
{
	return tal_uart_read(TUYA_UART_NUM_0, c, 1);
}

int uart_getline(const char *prompt, char *b, int len)
{
    if (NULL == b || 0 == len)
        return 0;

    char c;
    int i = 0;
    while (tal_uart_read(TUYA_UART_NUM_0, &c, 1) > 0)
    {
        b[i] = c;
        i++;

        if (c == '\n') {
            b[i] = '\0';
            PR_DEBUG("uart recv : %s", b);
            break;
        }

        if (i >= len) {
            break;
        }    
    }

    return i;    
}

int uart_tx_one_char(char c)
{
	return tal_uart_write(TUYA_UART_NUM_0, &c, 1);
}


/* uart thread handle */
// static THREAD_HANDLE uart_cmd_thread = NULL;

// void uart_cmd_task(void *pargs)
// {
//     while(1) {
// 		lua_handle_input(false);
// 	}
	
// 	tal_thread_delete(uart_cmd_thread);
// }

void uart_init(void) 
{
    TAL_UART_CFG_T cfg = {0};
    cfg.base_cfg.baudrate = 115200;
    cfg.base_cfg.databits = TUYA_UART_DATA_LEN_8BIT;
    cfg.base_cfg.stopbits = TUYA_UART_STOP_LEN_1BIT;
    cfg.base_cfg.parity = TUYA_UART_PARITY_TYPE_NONE;
    cfg.rx_buffer_size = 256;
    cfg.open_mode = O_BLOCK;
    int result = tal_uart_init(TUYA_UART_NUM_0,&cfg);
    if (OPRT_OK != result) {
        PR_ERR("uart init failed", result);
    }

    // THREAD_CFG_T thrd_param = {4096, 4, "uart_cmd_task"};
    // tal_thread_create_and_start(&uart_cmd_thread, NULL, NULL, uart_cmd_task, NULL, &thrd_param);
}
