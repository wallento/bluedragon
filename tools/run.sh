
qemu-system-or1k \
	-cpu or1200 \
	-M or1k-sim \
	-kernel linux/vmlinux \
	-serial stdio \
	-nographic \
	-monitor none
