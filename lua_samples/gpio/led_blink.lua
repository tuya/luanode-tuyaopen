-- LED blink 
-- The pin 2 connect to a LED. When the pin output high level, the LED will be light up,
-- otherwise, the LED will be turned off

isOff = false

led_blink = function()
  if(isOff) then
	gpio.write(2, gpio.HIGH);	-- turn on
	isOff = false
  else
	gpio.write(2, gpio.LOW);	-- turn off
	isOff = true
  end
end

gpio.open(2, gpio.OUTPUT, gpio.LOW);
tmr = timer.start(1000, timer.CYCLE, led_blink)

-- timer.stop(tmr)
-- timer.delete(tmr)
