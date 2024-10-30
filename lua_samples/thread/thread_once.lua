-- Thread run only once
-- The thread will exit when it run once

f1 = function()
  print("thread start");
  tmr.delay(1);		-- do somthing
  print("thread end");
end

thread.start(f1);
