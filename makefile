export PATH := $(CURDIR)/tools/toolchain/or1k-linux/bin:$(PATH)

export ARCH=openrisc

export CROSS_COMPILE=or1k-linux-

export CC=$(CURDIR)/tools/toolchain/or1k-linux-musl/bin/or1k-linux-musl-gcc


export OUTDIR=$(CURDIR)/linux/arch/openrisc/init

.PHONY: init

all: defconfig init
	cd linux && \
	$(MAKE)

defconfig:
	cd linux &&       \
	$(MAKE) defconfig

modules: defconfig
	cd linux &&       \
	$(MAKE) modules

init:
	mkdir -p $(OUTDIR)
	$(CC) init/*.c -static -o $(OUTDIR)/init
	

clean:
	rm -rf $(OUTDIR)/init
	cd linux &&       \
	$(MAKE) clean
