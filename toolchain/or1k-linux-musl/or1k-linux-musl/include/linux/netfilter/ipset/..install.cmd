cmd_/opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/netfilter/ipset/.install := /bin/bash scripts/headers_install.sh /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/netfilter/ipset ./include/uapi/linux/netfilter/ipset ip_set.h ip_set_bitmap.h ip_set_hash.h ip_set_list.h; /bin/bash scripts/headers_install.sh /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/netfilter/ipset ./include/linux/netfilter/ipset ; /bin/bash scripts/headers_install.sh /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/netfilter/ipset ./include/generated/uapi/linux/netfilter/ipset ; for F in ; do echo "\#include <asm-generic/$$F>" > /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/netfilter/ipset/$$F; done; touch /opt/crossbuild/output/or1k-linux-musl/or1k-linux-musl/include/linux/netfilter/ipset/.install
