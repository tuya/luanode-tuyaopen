-- connect GPIO5 to GPIO4
-- GPIO4 will be triggered when GPIO5 output different level

--   pin: pin to set
--   pin_type: the value can be: gpio.INPUT/gpio.OUTPUT/gpio.INT/gpio.INOUT
--   intr_type: how to trigger, the value is string, can be: "up"/"down"/"both"/"low"/"high"
--   callback: callback function

-- to remove a callback call: gpio.remove(pin)
-- to remove all GPIO ISR call: gpio.uninstall()

pin_intr = 4;
pin_out = 5;
callback = function()
  print("run callback");
end

-- gpio.open(pin, mode, value), value is default status
-- for OUTPUT, value can be: LOW, HIGH
-- for INPUT, value can be: PULLUP, PULLDOWN and FLOATING
gpio.open(pin_intr, gpio.INPUT, gpio.FLOATING)
gpio.open(pin_out, gpio.OUTPUT, gpio.LOW)

-- gpio.set_irq(pin, mode, callback)
-- mode support: IRQ_RISE, IRQ_FALL, IRQ_BOTH, IRQ_LOW, IRQ_HIGH
gpio.set_irq(pin_intr, gpio.IRQ_FALL, callback)
gpio.enable_irq(pin_intr)

-- delay and trigger the interrupt
sys.delay_ms(2000)
gpio.write(pin_out, gpio.HIGH)
sys.delay_ms(1000)
gpio.write(pin_out, gpio.LOW)

-- close the pin
gpio.close(pin_intr)
gpio.close(pin_out)