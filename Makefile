#
#  ターゲットの指定（Makefile.targetで上書きされるのを防ぐため）
#
all:

#
#  アプリケーションファイル
#
APPLNAME = sessalet
USE_CXX = true
APPL_CFG = $(APPLNAME).cfg

DEBUG = true
OMIT_OPTIMIZATION = true

#
#  MBEDライブラリのディレクトリの定義
#
MBED_LIB_DIR = ../asp-gr_peach_gcc-mbed/mbed-lib

#
#  ASPソースファイルのディレクトリの定義
#
SRCDIR = ../asp-gr_peach_gcc-mbed/asp-1.9.2-utf8

# CXX_OBJ += control.o \
#		   action.o
CXX_OBJ += $(MBED_LIB_DIR)/rohm-sensor-hal/source/I2CCommonMbedHardwareLib.o  $(MBED_LIB_DIR)/rohm-rpr0521/source/rpr0521_driver.o

#
#  MBEDライブラリのビルド
#
include $(MBED_LIB_DIR)/common/Makefile.cmn
include $(MBED_LIB_DIR)/mbed-src/Makefile.src
include $(MBED_LIB_DIR)/SoftPWM/Makefile.pwm
include $(MBED_LIB_DIR)/Motor/Makefile.mtr
#include $(MBED_LIB_DIR)/Rohm/Makefile.rhm
include $(MBED_LIB_DIR)/Makefile.mbd

INCLUDE_PATHS += -I$(MBED_LIB_DIR)/Motor \
	-I$(MBED_LIB_DIR)/rohm-sensor-hal \
	-I$(MBED_LIB_DIR)/rohm-rpr0521 \
	-I$(SRCDIR)/include \
	-I$(SRCDIR)/target/gr_peach_gcc \
	-I$(SRCDIR)/arch \
	-I$(SRCDIR)/arch/arm_gcc/common \


#
#  ASPカーネルライブラリ
#
KERNEL_LIB = .
include $(SRCDIR)/Makefile.asp
