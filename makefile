export PATH := $(CURDIR)/tools/toolchain/or1k-linux/bin:$(PATH)

export ARCH=openrisc

export CROSS_COMPILE=or1k-linux-

export CFLAGS_MODULE=-fno-pic

all: defconfig
	cd linux && \
	$(MAKE)

defconfig:
	cd linux &&       \
	$(MAKE) defconfig

modules: defconfig
	cd linux &&       \
	$(MAKE) modules

clean:
	cd linux &&       \
	$(MAKE) clean
