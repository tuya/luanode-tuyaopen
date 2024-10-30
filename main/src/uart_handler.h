#ifndef _UART_HANDLER_H_
#define _UART_HANDLER_H_


void uart_init(void);
int uart_getc(char *c);
int uart_getline(const char *prompt, char *c, int len);
int uart_tx_one_char(char c);
#endif
