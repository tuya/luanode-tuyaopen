# 当前文件所在目录
ifeq ($(CONFIG_ENABLE_FFI), y)
LOCAL_PATH := $(call my-dir)
#---------------------------------------
define get-parent-abs-dir
$(strip $(patsubst %/,%,$(dirname $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))))
endef
PARENT_ABS_DIR := $(strip $(patsubst %/,%,$(dirname $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))))
# 清除 LOCAL_xxx 变量
# include $(CLEAR_VARS)
#
LOCAL_PKG_NAME := $(notdir $(LOCAL_PATH))
# LOCAL_PKG_SRC := $(abspath $(LOCAL_PATH))
LOCAL_PKG_SRC :=
LOCAL_PKG_DEPS :=
LOCAL_PKG_EXPORT :=
LOCAL_PKG_OPTS := --disable-shared --enable-static --disable-docs --disable-exec-static-tramp --disable-raw-api
LOCAL_TUYA_SDK_INC := $(LOCAL_PATH)/include
TUYA_SDK_INC += $(LOCAL_TUYA_SDK_INC)  # 此行勿修改


# 模块的 CFLAGS
ifeq (,$(findstring -mslow-flash-data,$(TUYA_PLATFORM_CFLAGS)))
    $(info -mslow-flash-data not found in TUYA_PLATFORM_CFLAGS)
	LOCAL_CFLAGS := -fPIC
else
    $(info -mslow-flash-data found in TUYA_PLATFORM_CFLAGS)
	LOCAL_CFLAGS :=
endif
LOCAL_CFLAGS += -fomit-frame-pointer

ifneq ($(CONFIG_OPERATING_SYSTEM),100)
    LOCAL_CFLAGS+=-DNDEBUG
endif
LOCAL_CFLAGS += $(TUYA_PLATFORM_CFLAGS) -DNO_JAVA_RAW_API


PARENT_ABS_DIR := $(patsubst %/,%,$(dir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))))
PARENT_ABS_DIR := $(shell dirname $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST)))))
PARENT_ABS_DIR := $(shell dirname $(PARENT_ABS_DIR))

LOCAL_CFLAGS += -I${PARENT_ABS_DIR}/components/tal_system/include

ADAPTER_DIR:=${PARENT_ABS_DIR}/adapter/utilities/include
ifneq ("$(wildcard $(ADAPTER_DIR))","")
	LOCAL_CFLAGS += -I${ADAPTER_DIR}
endif
ADAPTER_DIR:=${PARENT_ABS_DIR}/vendor/$(TARGET_PLATFORM)/tuyaos/tuyaos_adapter/utilities/include
ifneq ("$(wildcard $(ADAPTER_DIR))","")
	LOCAL_CFLAGS += -I${ADAPTER_DIR}
endif

ADAPTER_DIR:=${PARENT_ABS_DIR}/vendor/$(TARGET_PLATFORM)/tuyaos/tuyaos_adapter/include/utilities/include
ifneq ("$(wildcard $(ADAPTER_DIR))","")
	LOCAL_CFLAGS += -I${ADAPTER_DIR}
endif

ADAPTER_DIR:=${PARENT_ABS_DIR}/adapter/system/include
ifneq ("$(wildcard $(ADAPTER_DIR))","")
	LOCAL_CFLAGS += -I${ADAPTER_DIR}
endif
ADAPTER_DIR:=${PARENT_ABS_DIR}/vendor/$(TARGET_PLATFORM)/tuyaos/tuyaos_adapter/system/include
ifneq ("$(wildcard $(ADAPTER_DIR))","")
	LOCAL_CFLAGS += -I${ADAPTER_DIR}
endif

ADAPTER_DIR:=${PARENT_ABS_DIR}/vendor/$(TARGET_PLATFORM)/tuyaos/tuyaos_adapter/include/system
ifneq ("$(wildcard $(ADAPTER_DIR))","")
	LOCAL_CFLAGS += -I${ADAPTER_DIR}
endif

LOCAL_CFLAGS += -I${PARENT_ABS_DIR}/include/base/include

exist = $(shell if [ ! -f ${LOCAL_PATH}/configure ]; then cd ${LOCAL_PATH}; ./autogen.sh; else echo exist;fi;)
ifneq ($(exist), "exist")
$(info "create configure")
endif

include $(BUILD_PACKAGE)
LOCAL_LIBFFI_PATH := $(call static-objects-dir)/$(LOCAL_PATH)
LIBFFI_OUT_INC_PATH := ${_LOCAL_PKG_OUT}/include
LIBFFI_INC_PATH := ${LOCAL_PATH}/include
libffi_static: $(_LOCAL_PKG_TARGET_BUILD)
	@echo "====== libffi ar -x ======="
	@if [ -f $(OUTPUT_DIR)/lib/libffi.a ]; then \
		echo $(LOCAL_LIBFFI_PATH); \
		mkdir -p $(LOCAL_LIBFFI_PATH); \
		cp ${LIBFFI_OUT_INC_PATH}/*.h -t ${LIBFFI_INC_PATH}; \
		cp $(OUTPUT_DIR)/lib/libffi.a $(LOCAL_LIBFFI_PATH); \
		cd $(LOCAL_LIBFFI_PATH); \
		$(AR) -x libffi.a; \
		rm libffi.a; fi
	@echo "------ libffi ar -x end ---------"

os_sub_static: libffi_static

libff_preconfig:$(_LOCAL_PKG_TARGET_CONF)
	cp ${LIBFFI_OUT_INC_PATH}/*.h -t ${LIBFFI_INC_PATH};

config:libff_preconfig
endif