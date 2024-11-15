
## 快速体验

#### 安装依赖

安装ubuntu系统，推荐20.04版本

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
