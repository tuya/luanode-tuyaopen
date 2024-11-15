-- luaproc: https://github.com/askyrme/luaproc/blob/master/README.md
-- Thread run only once
-- The thread will exit when it run once

f1 = function() 
  print("thread start")
  sys.delay_ms(5000)		-- do somthing
  print("thread end")
end

luaproc.newproc(f1)
