
#include "tal_sw_timer.h"
#include "lua_timer.h"
#include "lua_msgbus.h"
#include "tal_log.h"

#define TKL_TIMER_COUNT 32
static LUA_TIMER_T* timers[TKL_TIMER_COUNT] = {0};


static LUA_TIMER_T* lua_timer_get_with_os_timerid(TIMER_ID timer_id)
{
    for (size_t i = 0; i < TKL_TIMER_COUNT; i++)
    {
        if (timers[i] && timers[i]->os_timer == timer_id) {
            return timers[i];
        }
    }
    return NULL;
}

LUA_TIMER_T* lua_timer_get(int timer_id)
{
    for (size_t i = 0; i < TKL_TIMER_COUNT; i++)
    {
        if (timers[i] && timers[i]->id == timer_id) {
            return timers[i];
        }
    }
    return NULL;
}

static void lua_timer_callback(TIMER_ID os_timer, void *arg)
{
    //LLOGD("timer callback");
    TUYA_RTOS_MSG_T msg;
    LUA_TIMER_T *timer = lua_timer_get_with_os_timerid(os_timer);
    msg.handler = timer->func;
    msg.ptr = timer;
    msg.arg1 = timer->id;
    msg.arg2 = 0;
    lua_msgbus_put(&msg, 0);
}

static int nextTimerSlot()
{
    for (size_t i = 0; i < TKL_TIMER_COUNT; i++) {
        if (timers[i] == NULL) {
            return i;
        }
    }
    return -1;
}

int lua_timer_start(LUA_TIMER_T* timer,BOOL_T new)
{
    TIMER_ID os_timer = NULL;
    int timerIndex;

    if (new) {
        timerIndex = nextTimerSlot();
        if (timerIndex < 0) {
            PR_ERR("too many timers");
            return 1; // too many timer!!
        }
        tal_sw_timer_create(lua_timer_callback,NULL,&os_timer);
        if (!os_timer) {
            PR_ERR("tal_sw_timer_create FAIL");
            return -1;
        }
        timers[timerIndex] = timer;

        timer->os_timer = os_timer;
    }
    else {
        os_timer = lua_timer_get(timer->id);
    }


    int re = tal_sw_timer_start(os_timer,timer->timeout,timer->repeat);
    if (re != OPRT_OK) {
        PR_ERR("tal_sw_timer_start FAIL");
    }
    return re == OPRT_OK ? 0 : -1;
}

int lua_timer_stop(LUA_TIMER_T* timer)
{
    if (timer == NULL || timer->os_timer == NULL)
        return -1;


    int ret = tal_sw_timer_stop(timer->os_timer);
    if (ret == OPRT_OK) {
        timer->os_timer = NULL;
    }
    return 0;
}

int lua_timer_delete(LUA_TIMER_T* timer)
{
    int ret = tal_sw_timer_delete(timer->os_timer);
    if (ret == OPRT_OK) {
        for (size_t i = 0; i < TKL_TIMER_COUNT; i++) {
            if (timers[i] == timer) {
                timers[i] = NULL;
                break;
            }
        }
    }

    return ret;
}

int lua_timer_trigger(LUA_TIMER_T* timer)
{
    if (timer == NULL || timer->os_timer == NULL)
        return 1;

    tal_sw_timer_trigger(timer->os_timer);
    return 0;
}


