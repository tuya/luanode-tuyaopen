# ELUA CATX
  当前在CAT.1平台上支持LUA引擎。lua版本为5.3版本。
## 支持的模块
### 基础模块、coroutine、string、math、table
### 扩展模块
#### catx
##### 函数列表
    pdp_active
    pdp_get_status
    get_cfun
    set_cfun
    get_imei
    get_iccid
    get_rsrp
    get_rssi
    get_module_name
    get_sysfw_ver
    get_nettype
    sys_backup
    set_plmn
    get_net_connected
    process_pdp_event
    process_reg_event
    process_sim_event

#### ty_event
    本事件订阅只支持对多20个事件订阅
##### 函数列表
    ty_subscribe_event
    ty_publish_event
    ty_unsubscribe_event
##### 默认定义事件
    CELL_REGISTION_EVENT
    CELL_PDP_EVENT
    CELL_SIM_EVENT
    CELL_NET_ISSUE_EVENT
    EVENT_RESET
    EVENT_INIT
    EVENT_MQTT_CONNECTED
    EVENT_MQTT_DISCONNECTED
    EVENT_OTA_START_NOTIFY
    EVENT_OTA_PROCESS_NOTIFY
    EVENT_OTA_FAILED_NOTIFY
    EVENT_OTA_FINISHED_NOTIFY
##### 默认定义事件类型
    SUBSCRIBE_TYPE_NORMAL
    SUBSCRIBE_TYPE_EMERGENCY
    SUBSCRIBE_TYPE_ONETIME

#### tyos
##### 函数列表
    timer_create
    timer_start
    timer_stop
    timer_isrunning
    timer_trigger
    timer_delete
    msgbuf_recv
    msgbuf_send
    create_sem
    take_sem
    give_sem
    reboot
    poweron_reason
    buildDate
    setPaths
    sleep_ms
    get_sys_ms
    log
    get_random

#### tuya_kv
##### 函数列表
    kv_set
    kv_get
    kv_del

#### tuyaos
##### 函数列表


#### uart
##### 函数列表
    open
    write
    read
    close
    set_tx_irq
    set_rx_irq
    wait_rx
    ioctl
##### 默认串口IOCTL命令
    SUSPEND
    RESUME
    FLUSH
    RECONFIG
##### 默认串口属性
    NONE
    ODD
    EVEN

```
local port_id = 0
local cmd = uart.TUYA_UART_RECONFIG_CMD
local arg = {
    baudrate = 115200,
    parity = 0, -- Replace with the appropriate value
    databits = 8, -- Replace with the appropriate value
    stopbits = 1, -- Replace with the appropriate value
    flowctrl = 0 -- Replace with the appropriate value
}

local result = uart.ioctl(port_id, cmd, arg)

if result == 0 then
    print("Operation successful")
else
    print("Operation failed with error code:", result)
end
```
## luaproc
## API

**`luaproc.newproc( string lua_code )`**

**`luaproc.newproc( function f )`**

Creates a new Lua process to run the specified string of Lua code or the
specified Lua function. Returns true if successful or nil and an error message
if failed. The only libraries loaded in new Lua processes are luaproc itself and
the standard Lua base and package libraries. The remaining standard Lua
libraries (io, os, table, string, math, debug, coroutine and utf8) are
pre-registered and can be loaded with a call to the standard Lua function
`require`.

**`luaproc.setnumworkers( int number_of_workers )`**

Sets the number of active workers (pthreads) to n (default = 1, minimum = 1).
Creates and destroys workers as needed, depending on the current number of
active workers. No return, raises error if worker could not be created.

**`luaproc.getnumworkers( )`**

Returns the number of active workers (pthreads).

**`luaproc.wait( )`**

Waits until all Lua processes have finished, then continues program execution.
It only makes sense to call this function from the main Lua script. Moreover,
this function is implicitly called when the main Lua script finishes executing.
No return.

**`luaproc.recycle( int maxrecycle )`**

Sets the maximum number of Lua processes to recycle. Returns true if successful
or nil and an error message if failed. The default number is zero, i.e., no Lua
processes are recycled.

**`luaproc.send( string channel_name, msg1, [msg2], [msg3], [...] )`**

Sends a message (tuple of boolean, nil, number or string values) to a channel.
Returns true if successful or nil and an error message if failed. Suspends
execution of the calling Lua process if there is no matching receive.

**`luaproc.receive( string channel_name, [boolean asynchronous] )`**

Receives a message (tuple of boolean, nil, number or string values) from a
channel. Returns received values if successful or nil and an error message if
failed. Suspends execution of the calling Lua process if there is no matching
receive and the async (boolean) flag is not set. The async flag, by default, is
not set.

**`luaproc.newchannel( string channel_name )`**

Creates a new channel identified by string name. Returns true if successful or
nil and an error message if failed.

**`luaproc.delchannel( string channel_name )`**

Destroys a channel identified by string name. Returns true if successful or nil
and an error message if failed. Lua processes waiting to send or receive
messages on destroyed channels have their execution resumed and receive an error
message indicating the channel was destroyed.

## References

A paper about luaproc -- *Exploring Lua for Concurrent Programming* -- was
published in the Journal of Universal Computer Science and is available
[here](http://www.jucs.org/jucs_14_21/exploring_lua_for_concurrent) and
[here](http://www.inf.puc-rio.br/~roberto/docs/ry08-05.pdf). Some information in
the paper is already outdated, but it still provides a good overview of the
library and some of its design choices.

A tech report about concurrency in Lua, which uses luaproc as part of a case
study, is also available
[here](ftp://ftp.inf.puc-rio.br/pub/docs/techreports/11_13_skyrme.pdf).

Finally, a paper about an experiment to port luaproc to use Transactional Memory
instead of the standard POSIX Threads synchronization constructs, published as a
part of the 8th ACM SIGPLAN Workshop on Transactional Computing, can be found
[here](http://transact2013.cse.lehigh.edu/skyrme.pdf).