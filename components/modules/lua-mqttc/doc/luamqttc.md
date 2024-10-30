# MQTT Lua C 接口文档

## 1. 创建 MQTT 客户端
**new**
```lua
local mqtt_client = mqttc.new()
print("MQTT client created.")
```
## 2. 注册回调函数
**on**
注册不同的回调函数以处理 MQTT 事件。
**示例**
```lua
local function on_conack(success)
    if success == 1 then
        print("Connected successfully!")
    else if success == -1 then
        print("mqtt disconnect message.")
    else if success == -2 then
        print("Conentd to broker with error.")
    end
end

mqtt_client:on("conack", on_conack)
```

## 3. 发布消息
**publish(topic, payload,{qos=0})**
注册不同的回调函数以处理 MQTT 事件。
**示例**
```lua

local topic = "my/topic"
local payload = "Hello, MQTT!"
local function on_puback(ret,topic,payload)
    if ret == 0 then
        print("Publish success.")
    else
        print("Publish failed.")
    end
    print("topic:",topic)
    -- 检查 payload 的长度
    if payload then
        local payload_length = #payload  -- 获取 payload 的长度
        print("Payload length:", payload_length)

        -- 处理二进制数据
        -- 例如，将二进制数据转为十六进制字符串以便于阅读.如果是string类型，直接打印即可
        local hex_payload = ""
        for i = 1, payload_length do
            local byte = string.byte(payload, i)  -- 获取每个字节
            hex_payload = hex_payload .. string.format("%02X ", byte)  -- 转为十六进制
        end
        print("Payload (hex):", hex_payload)
    else
        print("No payload received.")
    end
end


local result = mqtt_client:publish(topic, payload,{qos=0})

if result then
    print("Message published successfully.")
else
    print("Failed to publish message.")
end

```

## 4. 订阅主题
**subscribe(topic)**
注册不同的回调函数以处理 MQTT 事件。
**示例**
```lua
local function on_message(topic, payload)
    print("Received message on topic: " .. topic)
    print("Payload: " .. payload)
end

mqtt_client:on("message", on_message)

local success = mqtt_client:subscribe({"my/topic","my/topic1"})
if success then
    print("Subscribed to topic successfully.")
else
    print("Failed to subscribe to topic.")
end


```

## 5. 订阅主题
**unsubscribe(topic)**
注册不同的回调函数以处理 MQTT 事件。
**示例**
```lua

local success = mqtt_client:unsubscribe({"my/topic","my/topic1"})
if success then
    print("Unsubscribed from topic successfully.")
else
    print("Failed to unsubscribe from topic.")
end


```

## 6. 连接到 MQTT 代理
**connect(options)**
连接到指定的 MQTT 代理
**示例**
```lua

local connect_options = {
    host = "mqtt.example.com",
    port = 1883,
    client_id = "my_client_id",
    username = "my_username",
    password = "my_password",
    keep_alive = 60
}

local success = mqtt_client:connect(connect_options)
if success then
    print("Connected to MQTT broker.")
else
    print("Failed to connect to MQTT broker.")
end

```


## 7. 获取 MQTT 客户端版本
**version()**
获取当前 MQTT 客户端的版本信息。
**示例**
```lua

local version = mqtt_client:version()
print("MQTT Client Version: " .. version)


```

## 8. 获取返回代码字符串
**return_code_string(code)**
将返回的代码转换为字符串描述。
**示例**
```lua

local code = -1 -- 假设这是返回的代码
local message = mqtt_client:return_code_string(code)
print("Return Code Message: " .. message)


```