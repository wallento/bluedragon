cmd_/opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/can/.install := /bin/bash scripts/headers_install.sh /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/can ./include/uapi/linux/can bcm.h error.h gw.h netlink.h raw.h; /bin/bash scripts/headers_install.sh /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/can ./include/linux/can ; /bin/bash scripts/headers_install.sh /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/can ./include/generated/uapi/linux/can ; for F in ; do echo "\#include <asm-generic/$$F>" > /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/can/$$F; done; touch /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/can/.install
