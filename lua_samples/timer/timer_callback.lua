-- Register call back for a timer
-- Timer trigger each period, change parameter to tmr.ALARM_SINGLE to trigger once
a={v=0}
callback = function(a)  print("times up ".. a.v) a.v= a.v+1 end

-- start a timer, callback once after 5000ms
-- timer.start(timerout_in_ms, timer.ONCE or timer.CYCLE, callback, parameter for callback)
tmr = timer.start(5000, 1, callback, a)

-- trigger timer immideately
timer.trigger(tmr)

-- stop timer
timer.stop(tmr)

-- delete timer
timer.delete(tmr)

