-- Thread test
-- Start a thread for LED blinking
-- Print status of the thread

f1=function()
  while true do
    gpio.write(2,0);
    tmr.delay(1);
    gpio.write(2,1);
    tmr.delay(1);
  end
end

th1=thread.start(f1);
--status = thread.status(th1);  -- get thread status, and then print it
--print(status);	-- this will output running
