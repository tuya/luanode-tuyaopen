-- Register call back for a timer
-- Timer trigger each period, change parameter to tmr.ALARM_SINGLE to trigger once

callback = function(id)
  print("times up!");
end

-- set timer id and callback period
tmrId = 1;
period = 1000000;	-- call callback each 1s, unit is microsecond

tmr.register(tmrId, period, tmr.ALARM_SINGLE, callback); -- to setup a loop timer, used tmr.ALARM_PERIODIC as 3th parameter
tmr.start(tmrId);	-- start timer

-- to stop the timer, run the following command
-- tmr.stop(tmrId);


-- to make the timer trigger only once, modify the third parameter of tmr.register as below
-- tmr.register(tmrId, period, tmr.ALARM_SINGLE, callback);

