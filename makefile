export PATH := $(CURDIR)/tools/toolchain/or1k-linux/bin:$(PATH)

export ARCH=openrisc

export CROSS_COMPILE=or1k-linux-

export CC=$(CURDIR)/tools/toolchain/or1k-linux-musl/bin/or1k-linux-musl-gcc


export OUTDIR=$(CURDIR)/linux/arch/openrisc/initramfs

.PHONY: initramfs

all: defconfig initramfs
	cd linux && \
	$(MAKE)

defconfig:
	cd linux &&       \
	$(MAKE) defconfig

modules: defconfig
	cd linux &&       \
	$(MAKE) modules

initramfs:
	mkdir -p $(OUTDIR)
	$(CC) initramfs/*.c -static -o $(OUTDIR)/init
	

clean:
	rm -rf $(OUTDIR)/init
	cd linux &&       \
	$(MAKE) clean
