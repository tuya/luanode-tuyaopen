/*=========================================================================*\
* Input/Output abstraction
* LuaSocket toolkit
\*=========================================================================*/
#include "luasocket.h"
#include "io.h"

/*-------------------------------------------------------------------------*\
* Initializes C structure
\*-------------------------------------------------------------------------*/
void io_init(p_io io, p_send send, p_recv recv, p_error error, void *ctx) {
    io->send = send;
    io->recv = recv;
    io->error = error;
    io->ctx = ctx;
}

/*-------------------------------------------------------------------------*\
* I/O error strings
\*-------------------------------------------------------------------------*/
const char *io_strerror(int err) {
    switch (err) {
        case IO_DONE: return NULL;
        case IO_CLOSED: return "closed";
        case IO_TIMEOUT: return "timeout";
        case IO_UNSUPPORTED:return "unsupport";
        default: return "unknown error";
    }
}
