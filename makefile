export PATH := $(CURDIR)/tools/toolchain/or1k-linux/bin:$(PATH)

export ARCH=openrisc

export CROSS_COMPILE=or1k-linux-

all: defconfig
	cd linux && \
	$(MAKE)

defconfig:
	cd linux &&       \
	$(MAKE) defconfig

clean:
	cd linux &&       \
	$(MAKE) clean
