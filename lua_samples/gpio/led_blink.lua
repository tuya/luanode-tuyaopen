-- LED blink 
-- The pin 2 connect to a LED. When the pin output high level, the LED will be light up,
-- otherwise, the LED will be turned off

isOff = false

led_blink = function()
  if(isOff) then
	gpio.write(2, 1);	-- turn on
	isOff = false
  else
	gpio.write(2, 0);	-- turn off
	isOff = true
  end
end

--gpio.mode(2,gpio.OUTPUT);

period = 1000;
tmr.register(1, period, tmr.ALARM_AUTO, led_blink);
tmr.start(1);
