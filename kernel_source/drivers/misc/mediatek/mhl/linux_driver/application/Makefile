#
# Copyright (C) 2011-2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify it under the terms of the
# GNU General Public License version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <http://www.gnu.org/licenses/>.
#
#
# Makefile for the Silicon Image 835x MHL TX device driver
# test application
#

OBJS += \
testSii8338Drv.o \
driver_event.o

INCLUDES := -I/usr/loca/angstrom/arm/arm-angstrom-linux-gnueabi/usr/include
INCLUDES += -I../linux_kernel/public

CROSS_COMPILE :=/usr/local/angstrom/arm/bin/arm-angstrom-linux-gnueabi-
CC     = $(CROSS_COMPILE)gcc
CXX    = $(CROSS_COMPILE)g++
LD     = $(CROSS_COMPILE)ld
AR     = $(CROSS_COMPILE)ar
STRIP  = $(CROSS_COMPILE)strip
NM     = $(CROSS_COMPILE)nm

PWD := $(shell pwd)
PATH := $(COMPILE_TOOLS):$(PATH)
WORK_DIR := /opt/WorkDir/sii8338
.PHONY:	clean

all:		testSii8338Drv
		@echo 'make all complete'

clean:
		rm -rf testSii8338Drv *.o *.d
		@echo 'make complete'

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
#	arm-angstrom-linux-gnueabi-gcc -I/usr/loca//angstrom/arm/arm-angstrom-linux-gnueabi/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	$(CC) $(INCLUDES) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


testSii8338Drv: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CC) -lrt -o"testSii8338Drv" $(OBJS)
	cp -vf ./testSii8338Drv $(WORK_DIR)
	@echo 'Finished building target: $@'
	@echo ' '
