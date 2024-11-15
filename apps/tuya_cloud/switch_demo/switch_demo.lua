-- a simple switch demo
-- swith have only 1 dp, default is false
switch_dps = {}
switch_dps[1] = {dpid=1, type=tuyaos.PROP_BOOL, value=false}

-- set meta info
-- note: 
--   Visit https://platform.tuya.com/ to create the PID
--   Visit https://platform.tuya.com/purchase/index?type=6 to get the open-sdk uuid and authkey.
tuyaos.meta({pid="qhivvyqawogv04e4", uuid="uuid498efebcca296998", authkey="IbOyiGV8Yuq8sjIxUp3a2ajKCSmnBCnp"})

-- on device binding
-- translate shorturl to QRCode (use: https://cli.im/) 
-- scan the QRCdoe with tuya smartlife APP to bind it to your account 
-- it used for platform ubuntu, on t1/t2/t3/t5/esp32/esp32c3, it not needed
tuyaos.on("shorturl", function (url) print("-----shorturl is------") print(url) print("-----end------") end)

-- on receive app contorl
-- objdp is bool/value/string/enum/bitmap
function on_objdp_proc(objdp) 
    print("recv obj dp cnt:"..objdp.dps_cnt)
    for k, v in pairs(objdp.dps) do
        print(k,v.dpid, v.type, v.value) 
        switch_dps[v.dpid].value = v.value -- update local status, you can add you hardware process here
        tuyaos.report_obj(objdp.dps, objdp.dps_cnt) -- report to cloud
    end
end
tuyaos.on("objdp", on_objdp_proc)

-- on receive app control
-- raw dp is binary dp
-- tbd...

-- start the device
tuyaos.start_device()

-- after device startup, activate. report status to cloud
tuyaos.report_obj({switch_dps[1]}, 1)