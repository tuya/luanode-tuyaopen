## luanode-tuyaopen: 让IoT开发更简单 

luanode-tuyaopen 是一个开源的 IoT 开发框架，支持开发者使用 [Lua](https://github.com/lua/lua)  语言简单的，快速的开发 IoT 产品。它基于 [tuyaopen](https://github.com/tuya/tuyaopen) 构建，继承了其跨平台、跨系统、组件化的特性，可以很容易的在不同的操作系统、芯片上进行切换和移植。

luanode-tuyaopen 目前支持的目标板包含：

| platform |                            [main]                            |
| :------- | :----------------------------------------------------------: |
| 🧩ubuntu  | [![](https://img.shields.io/badge/supported-green.svg)][支持] |
| 🧩t1      | [![](https://img.shields.io/badge/underdevelopment-yellow.svg)][开发中] |
| 🧩t2      | [![](https://img.shields.io/badge/supported-green.svg)][支持] |
| 🧩t3      | [![](https://img.shields.io/badge/supported-green.svg)][支持] |
| 🧩t5      | [![](https://img.shields.io/badge/supported-green.svg)][支持] |
| 🧩ESP32      | [![](https://img.shields.io/badge/underdevelopment-yellow.svg)][开发中] |
| 🧩ESP32-c2/c3     | [![](https://img.shields.io/badge/underdevelopment-yellow.svg)][开发中] |


luanode-tuyaopen 是 tuyaopen framework 开发生态的组成部分，目前已经发布的开发资料还包含：

* [🚀TuyaOpen](https://github.com/tuya/tuyaopen): 开源的IoT 开发框架，帮助开发者快速实现产品的智能化。  
* [🚀TuyaOpen for Arduino](https://github.com/tuya/arduino-tuyaopen): 支持在 Arduino IDE 下使用 TuyaOpen 进行 IoT产品开发。



## 快速体验

#### 安装依赖

Install the Ubuntu system, version 20.04 is recommended.

```
$ sudo apt-get install lcov cmake-curses-gui build-essential wget git python3 python3-pip python3-venv libc6-i386 libsystemd-dev
```



#### 克隆仓库

```
$ git clone https://github.com/tuya/luanode-tuyaopen.git
$ git submodule update --init
```



#### 设置环境变量

```
$ cd tuyaopen
$ export PATH=$PATH:$PWD
```

或将 [tuyaopen](https://github.com/tuya/tuyaopen) 路径添加到系统环境变量中。 [tuyaopen](https://github.com/tuya/tuyaopen)  通过  [tos ](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md) 命令进行编译、调试等操作， [tos ](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md) 命令会根据环境变量中设置的路径查找  [tuyaopen](https://github.com/tuya/tuyaopen)  仓库，并执行对应操作。

 [tos ](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md) 命令的详细使用方法，请参考 [tos 命令](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md)。



#### 编译基础固件

使用 [Lua](https://github.com/lua/lua) 开发需要一个基础固件，该基础固件提供了 [tuyaopen](https://github.com/tuya/tuyaopen) 基础能力以及  [Lua](https://github.com/lua/lua) 引擎能力，我们需要在工程内部将其编译并烧录到目标板上。

```shell
$ cd mian
$ tos build
```

编译完成之后固件位于此目录之下，通过 [tos ](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md) 命令进行烧录。

```shell
$ cd .build/main/bin/
$ ls
-rwxrwxr-x  1 tuyaos tuyaos 4977400 Nov  5 13:35 main_1.0.0*
```

##### 修改目标板

如果需要修改目标板，需要修改工程目录的 `project_build.ini` 文件，目前支持的目标板有`ubuntu`、`t1`、`t2`、 `t3`、 `t5`。修改完成之后，需要删除`.build`目录，重新编译即可。

```ini
[project:main]
platform = ubuntu
```



#### 快速体验

固件烧录完成之后，重新启动即可进入 [Lua](https://github.com/lua/lua) 基础固件模式。在`ubuntu` 目标板，可以通过`stdin/stdout` 进行输入输出，直接接受 [Lua](https://github.com/lua/lua) 指令；在其他目标板上，可以通过串口工具，连接串口0进行输入输出直接接受 [Lua](https://github.com/lua/lua) 指令。

```shell
[MEM DBG] heap init-------size:524288 addr:0x7ff04ca7f800---------
[01-01 05:09:26 ty I][main.c:37] lfs init
[01-01 05:09:26 ty I][main.c:20] Create init.lua
[01-01 05:09:26 ty I][main.c:29] File written
Lua 5.3.6  Copyright (C) 1994-2020 Lua.org, PUC-Rio
> print("hello world")
hello world
> 
```



## 编写代码

luanode-tuyaopen的编码只支持 [Lua](https://github.com/lua/lua) 语言。以下是一个简单的示例：

```lua
-- simple mqtt client
c = mqtt.new() 
c:on("conack", function () print("lua recv connect ack") end)
c:on("disconack", function () print("lua recv dis-connect ack") end)
c:connect({host="broker.emqx.io", port=1883, client_id="tuyaopen-01", user_name="emqx", passwd="public"})
```

更多示例请参考项目`/lua_examples/`文件夹中的示例。



## Nodemcu-uploader工具

支持[nodemcu-uploader](https://github.com/kmpm/nodemcu-uploader)工具，并进行了一些个性化的适配修改。支持通过串口连接目标板，帮助开发者在开发过程中进行文件上传、下载、备份；也支持直接在本地、目标板上运行文件；同时支持对获取目标板基础固件信息、资源信息等。

```shell
$ python3 nodemcu-uploader.py --help
usage: nodemcu-uploader [-h] [--verbose] [--silent] [--version] [--port PORT] [--baud BAUD] [--start_baud START_BAUD] [--timeout TIMEOUT]
                        [--autobaud_time AUTOBAUD_TIME]
                        {backup,upload,exec,download,file,sys,terminal,port} ...
```



## 贡献代码

如果你对此项目感兴趣，并希望参与项目的共同建设，并成为一个代码贡献者，可以参考[Contirbution Guide](https://github.com/tuya/tuyaopen/blob/master/docs/en/contribute_guide.md)。



## 免责声明

用户应明确知晓，本项目可能包含由第三方开发的子模块（submodules），这些子模块可能独立于本项目进行更新。鉴于这些子模块的更新频率不受控制，本项目无法确保这些子模块始终为最新版本。因此，用户在使用本项目时，若遇到与子模块相关的问题，建议自行根据需要进行更新或于本项目提交问题（issue）。

若用户决定将本项目用于商业目的，应充分认识到其中可能涉及的功能性和安全性风险。在此情况下，用户应对产品的所有功能性和安全性问题承担全部责任，应进行全面的功能和安全测试，以确保其满足特定的商业需求。本公司不对因用户使用本项目或其子模块而造成的任何直接、间接、特殊、偶然或惩罚性损害承担责任。

## License

Apache 2.0 @ http://www.apache.org/licenses/LICENSE-2.0