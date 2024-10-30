#include "tal_queue.h"
#include "lua_msgbus.h"
static QUEUE_HANDLE s_queue = NULL;


/*
在 FreeRTOS 中，直接从 Lua 脚本注册中断到驱动可能会带来以下后果：

性能影响：Lua 是一种解释型脚本语言，其执行速度通常比编译型语言（如 C/C++）慢。将中断处理程序（ISR）直接注册到 Lua 脚本可能会导致 ISR 的执行速度变慢，从而影响整个系统的性能。

实时性影响：在实时操作系统（如 FreeRTOS）中，中断处理程序需要尽快执行并返回，以确保系统的实时性。由于 Lua 脚本的执行速度较慢，直接在 Lua 中处理中断可能会导致实时性降低。

内存消耗：Lua 使用垃圾回收机制来管理内存，这可能会导致内存碎片和额外的内存开销。在资源受限的嵌入式系统中，这可能会导致内存不足的问题。

中断安全问题：在中断处理程序中，通常需要遵循一些特定的编程规则，以确保中断安全。在 Lua 脚本中处理中断可能会导致意外的行为，例如死锁、数据竞争等。

调试困难：在 Lua 脚本中处理中断可能会使得调试变得更加困难，因为你需要同时跟踪 C/C++ 代码和 Lua 脚本的执行。

为了避免这些问题，建议在 C/C++ 代码中处理中断，并在需要时通过回调函数或其他机制将事件传递给 Lua 脚本。这样，你可以充分利用 C/C++ 代码的性能优势，同时保持系统的实时性和稳定性。

所以我们这里使用发送消息给lua，以便提升性能
 */


void lua_msgbus_init(void) {
    if (!s_queue) {
        tal_queue_create_init(&s_queue,sizeof(TUYA_RTOS_MSG_T),256);
    }
}
OPERATE_RET lua_msgbus_put(TUYA_RTOS_MSG_T* msg, size_t timeout) {
    if (s_queue == NULL)
        return 1;
    return tal_queue_post(s_queue,msg,timeout);
}
OPERATE_RET lua_msgbus_get(TUYA_RTOS_MSG_T* msg, size_t timeout) {
    if (s_queue == NULL)
        return 1;
    return tal_queue_fetch(s_queue,msg,timeout);
}
