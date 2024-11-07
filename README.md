## luanode-tuyaopen: make IoT easy 

luanode-tuyaopen is an open-source IoT development framework that allows developers to easily and quickly develop IoT products using the Lua language. It is built on top of the tuyaopen, inheriting its cross-platform, cross-system, and modular features, making it easy to switch and port across different operating systems and chips.

Currently, luanode-tuyaopen supports the following target platforms:

| platform |                            [main]                            |
| :------- | :----------------------------------------------------------: |
| 🧩ubuntu  | [![](https://img.shields.io/badge/supported-green.svg)][supported] |
| 🧩t1      | [![](https://img.shields.io/badge/underdevelopment-yellow.svg)][under-development] |
| 🧩t2      | [![](https://img.shields.io/badge/supported-green.svg)][supported] |
| 🧩t3      | [![](https://img.shields.io/badge/supported-green.svg)][supported] |
| 🧩t5      | [![](https://img.shields.io/badge/supported-green.svg)][supported] |
| 🧩ESP32      | [![](https://img.shields.io/badge/underdevelopment-yellow.svg)][under-development] |
| 🧩ESP32-c2/c3     | [![](https://img.shields.io/badge/underdevelopment-yellow.svg)][under-development] |




luanode-tuyaopen is a component of the Tuya open SDK development ecosystem. The currently released development resources also include:

* [🚀TuyaOpen](https://github.com/tuya/tuyaopen): An open-source IoT development framework that helps developers quickly implement product smartification.
* [🚀TuyaOpen for Arduino](https://github.com/tuya/arduino-tuyaopen): Supports the use of Tuya Open SDK for IoT product development within the Arduino IDE.


## Quick Start

#### Dependences

Install the Ubuntu system, version 20.04 is recommended.

```
$ sudo apt-get install lcov cmake-curses-gui build-essential wget git python3 python3-pip python3-venv libc6-i386 libsystemd-dev
```

#### Clone Repository

```
$ git clone https://github.com/tuya/luanode-tuyaopen.git
$ git submodule update --init
```

#### Enviroment Setting

```
$ cd tuyaopen
$ export PATH=$PATH:$PWD
```
Or add the [tuyaopen](https://github.com/tuya/tuyaopen) path to the system environment variables. The [tuyaopen](https://github.com/tuya/tuyaopen) is compiled, debugged, and operated through the tos tool. For the detail commands and usages, please refer to [tos commands](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md).



#### Build the firmware

Use tos tool to build the luanode-tuyaopen firmware, and burning it to the target platform.

```shell
$ cd mian
$ tos build
```

##### Change the platform

If you want to change the target platform, you need to change the `project_build.ini` under `main` directory. After that, remove `.build` and re-compile the project.

```ini
[project:main]
platform = ubuntu
```

#### Flash the firmware

After compile passed, find the firmware under `.build/mian/bin/` directory，download it to the target board through [tos commands](https://github.com/tuya/tuyaopen/blob/master/docs/zh/tos_guide.md)。

```shell
$ cd .build/main/bin/
$ ls
-rwxrwxr-x  1 tuyaos tuyaos 4977400 Nov  5 13:35 main_1.0.0*
```

#### Quick start

After download the firmare, restart the board and firmware will be run. if your target platform is `ubuntu`, the default I/O is stdin/stdout. on the other target paltform, the default I/O is UART 0.

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



## Programing

luanode-tuyaopen only support [Lua](https://github.com/lua/lua) language. below is a simple example:

```lua
-- simple mqtt client
c = mqtt.new() 
c:on("conack", function () print("lua recv connect ack") end)
c:on("disconack", function () print("lua recv dis-connect ack") end)
c:connect({host="broker.emqx.io", port=1883, client_id="tuyaopen-01", user_name="emqx", passwd="public"})
```

more examples please refer to `/lua_examples/` directory.



## Nodemcu-uploader tool

Support [nodemcu-uploader](https://github.com/kmpm/nodemcu-uploader) tool, and some personalized adaptations and modifications have been made. It supports connecting to the target board via a serial port, helping developers upload, download, and back up files during the development process. It also supports running files directly on the local machine or the target board, as well as obtaining basic firmware and resource information from the target board.

```shell
$ python3 nodemcu-uploader.py --help
usage: nodemcu-uploader [-h] [--verbose] [--silent] [--version] [--port PORT] [--baud BAUD] [--start_baud START_BAUD] [--timeout TIMEOUT]
                        [--autobaud_time AUTOBAUD_TIME]
                        {backup,upload,exec,download,file,sys,terminal,port} ...
```

## Contribute Code

If you are interested in the tuyaopen and wish to contribute to its development and become a code contributor, please first read the [Contirbution Guide](https://github.com/tuya/tuyaopen/blob/master/docs/en/contribute_guide.md)。



## Disclaimer and Liability Clause

Users should be clearly aware that this project may contain submodules developed by third parties. These submodules may be updated independently of this project. Considering that the frequency of updates for these submodules is uncontrollable, this project cannot guarantee that these submodules are always the latest version. Therefore, if users encounter problems related to submodules when using this project, it is recommended to update them as needed or submit an issue to this project.

If users decide to use this project for commercial purposes, they should fully recognize the potential functional and security risks involved. In this case, users should bear all responsibility for any functional and security issues, perform comprehensive functional and safety tests to ensure that it meets specific business needs. Our company does not accept any liability for direct, indirect, special, incidental, or punitive damages caused by the user's use of this project or its submodules.

## License

Apache 2.0 @ http://www.apache.org/licenses/LICENSE-2.0