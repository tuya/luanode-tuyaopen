-- luaproc: https://github.com/askyrme/luaproc/blob/master/README.md
-- Start a thread for LED blinking
-- Print status of the thread

f1=function()
  while true do
    gpio.write(2,0)
    sys.delay_ms(1000)
    gpio.write(2,1)
    sys.delay_ms(1000)
  end
end

-- add new workers for this function
cnt = luaproc.getnumworkers()
luaproc.setnumworkers(cnt + 1)
luaproc.newproc(f1);