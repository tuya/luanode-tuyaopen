##
# @file CMakeLists.txt
# @brief 
#/
# APP_PATH
set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})

# APP_NAME
get_filename_component(APP_NAME ${APP_PATH} NAME)
get_filename_component(TARGET_PLATFORM ${PLATFORM_PATH} NAME)
string(TIMESTAMP CURRENT_TIME "%Y-%m-%d %H:%M:%S")

# 添加外部项目 libffi
include(ExternalProject)

# 设置安装和包含路径
set(EXTERNAL_SRC_DIR "${APP_PATH}/../components/modules/libffi")
set(EXTERNAL_INSTALL_DIR "${APP_PATH}/.build/${APP_NAME}/lib")
if (CONFIG_OPERATING_SYSTEM EQUAL 100)
    set(EXTERNAL_TARGET "x86_64-linux-gnu")
    set(EXTERNAL_INCLUDE_DIR "-I${APP_PATH}/../tuya-open-sdk-for-device/src/common/include -I${APP_PATH}/.build/${APP_NAME}/include -Os -fomit-frame-pointer -DNO_JAVA_RAW_API")
else()
    get_filename_component(CC_NAME ${CMAKE_C_COMPILER} NAME)
    if(CC_NAME MATCHES "^([^-]+-[^-]+-[^-]+)-gcc$")
        # Extract the triplet
        set(HOST_NAME "${CMAKE_MATCH_1}")
    else()
        message(FATAL_ERROR "Unable to extract host triplet from compiler name: ${CC_NAME}")
    endif()
    set(EXTERNAL_TARGET "${HOST_NAME}")
    set(EXTERNAL_INCLUDE_DIR "-I${PLATFORM_PATH}/tuyaos/tuyaos_adapter/include/utilities/include -I${PLATFORM_PATH}/tuyaos/tuyaos_adapter/include/system -I${APP_PATH}/../tuya-open-sdk-for-device/src/common/include -I${APP_PATH}/../tuya-open-sdk-for-device/src/tal_system/include -I${APP_PATH}/.build/${APP_NAME}/include -Os -fomit-frame-pointer -DNO_JAVA_RAW_API --specs=nosys.specs ${CMAKE_C_FLAGS}")
endif()

ExternalProject_Add(libffi
    SOURCE_DIR ${EXTERNAL_SRC_DIR}
    BINARY_DIR ${EXTERNAL_SRC_DIR}/build
    CONFIGURE_COMMAND env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CFLAGS=${EXTERNAL_INCLUDE_DIR} ${EXTERNAL_SRC_DIR}/configure --prefix=${EXTERNAL_INSTALL_DIR} --host=${EXTERNAL_TARGET}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
include_directories(${EXTERNAL_SRC_DIR}/include)
link_directories(${EXTERNAL_INSTALL_DIR})

# current project
# APP_SRCS
file(GLOB_RECURSE APP_SRCS  ${APP_PATH}/src/*.c)
file(GLOB_RECURSE LUA_SRCS ${APP_PATH}/../components/elua/src/*.c)
# file(GLOB_RECURSE LUA_FFI_SRCS ${APP_PATH}/../components/elua/plugin/cffi-lua/src/*.cc)
file(GLOB_RECURSE LUA_PROC ${APP_PATH}/../components/modules/lua-proc/*.c)
file(GLOB_RECURSE LUA_CJSON ${APP_PATH}/../components/modules/lua-cjson/*.c)
file(GLOB_RECURSE LUA_TLS ${APP_PATH}/../components/modules/lua-mbedtls/src/*.c)
# file(GLOB_RECURSE LUA_MQTTC ${APP_PATH}/../components/modules/lua-mqttc/src/*.c)
# file(GLOB_RECURSE LUA_SOCKET ${APP_PATH}/../components/modules/lua-socket/src/*.c)
file(GLOB_RECURSE LUA_TUYAOPEN ${APP_PATH}/../components/modules/lua-tuyaopen/src/*.c)


list(APPEND APP_SRCS ${LUA_SRCS})
# list(APPEND APP_SRCS ${LUA_FFI_SRCS})
list(APPEND APP_SRCS ${LUA_PROC})
list(APPEND APP_SRCS ${LUA_CJSON})
list(APPEND APP_SRCS ${LUA_TLS})
# list(APPEND APP_SRCS ${LUA_MQTTC})
# list(APPEND APP_SRCS ${LUA_SOCKET})
list(APPEND APP_SRCS ${LUA_TUYAOPEN})

add_definitions(-DSTATIC_IN_RELEASE=static)
add_definitions(-DMAJOR_VERSION=4 -DMINOR_VERSION=1 -DMICRO_VERSION=1 -DVERSION=\"4.1.1\")
add_definitions(-DEXAMPLE_VER="${EXAMPLE_VER}" -DEXAMPLE_NAME="${EXAMPLE_NAME}" -DPLATFORM_NAME="${PLATFORM_NAME}" -DPLATFORM_CHIP="${PLATFORM_CHIP}" -DBUILD_DATE="${CURRENT_TIME}")

# APP_INC
set(APP_INC ${APP_PATH}/src)
list(APPEND APP_INC ${APP_PATH}/../components/elua/include)
list(APPEND APP_INC ${APP_PATH}/../components/elua/plugin/cffi-lua/src)
list(APPEND APP_INC ${APP_PATH}/../components/modules/lua-proc)
list(APPEND APP_INC ${APP_PATH}/../components/modules/lua-cjson)
list(APPEND APP_INC ${APP_PATH}/../components/modules/lua-mbedtls)
list(APPEND APP_INC ${APP_PATH}/../components/modules/lua-mqttc)
list(APPEND APP_INC ${APP_PATH}/../components/modules/lua-socket)
list(APPEND APP_INC ${APP_PATH}/../components/modules/lua-tuyaopen/include)

# SUB_DIR
# add_subdirectory(${APP_PATH}/../components/lua ${APP_PATH}/.build/component/lua)

########################################
# Target Configure
########################################
add_library(${EXAMPLE_LIB})
add_dependencies(${EXAMPLE_LIB} libffi)
add_definitions(-DCFFI_LUA_DLL  -DFFI_LITTLE_ENDIAN)

target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_SRCS}
    )

target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_INC}
    )
