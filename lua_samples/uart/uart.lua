-- The uart module provide the following method
--
-- 1. uart.open(id, baud, [databits], [parity], [stopbits], [flow_ctrl], [txd], [rxd], [rts], [cts])
--      id: the uart number
--      baud: the uart baud rate
--      databits: can be the following constants: uart.DATA_8_BITS/uart.DATA_7_BITS/uart.DATA_6_BITS/uart.DATA_5_BITS
--      parity: can be uart.PARITY_NONE/uart.PARITY_ODD/uart.PARITY_EVEN
--      stopbits: can be uart.STOPBITS_1/uart.STOPBITS_1_5/uart.STOPBITS_2
--      flow_ctrl: flow control enable. normally set to uart.FLOWCTRL_NONE
--      txd: pin of txd, optional, default pin is GPIO4
--      rxd: pin of rxd, optional, default pin is GPIO5
--      rts: pin of rts, optional, default pin is GPIO18
--      cts: pin of cts, optional, default is GPIO19
-- 2. uart.write(id, string1, [string2], ..., [stringn])
-- 3. uart.read(id)
-- 3. uart.close(id)

uart.open(uart.UART_0, 115200, uart.DATA_8_BITS, uart.PARITY_NONE, uart.STOPBITS_1, uart.FLOWCTRL_NONE)
while true do
  sys.delay(2);
  uart.write(uart.UART_0, "hello");
end

uart.close(uart.UART_0)