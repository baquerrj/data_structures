###################################
# Makefile for Kernel Timer Module
###################################

LINUXDIR := $(HOME)/boulder/buildroot/output/build/linux-4796173fc58688055a99a1cef19a839174067220

obj-m := animals.o
all:
	make ARCH=arm CROSS_COMPILE=arm-buildroot-linux-gnueabihf- \
		-C $(LINUXDIR) M=$(shell pwd)
clean:
	make -C $(LINUXDIR) M=$(shell pwd) clean
