PKG=etherboot
VER=5.4.3
D=${PKG}-${VER}


HI=bootp.h dev.h elf_boot.h elf.h if_arp.h if_ether.h in.h ip.h isa.h little_bswap.h nic.h pci.h pci_ids.h tcp.h tftp.h udp.h disk.h
HB=cpu.h elf.h byteswap.h


CC=misc.c nic.c pci.c pci_probe.c proto_eth_slow.c disk.c

CDN=rtl8139.c
CDD=filo.c ide_disk.c

ALL=patch.stamp timer.h etherboot.h patch.stamp

default:done.stamp

done.stamp:${ALL}
	touch $@

patch.stamp:  etherboot.patch
	/bin/rm -rf $D
	tar xfj ${PKG}-${VER}.tar.bz2
	for i in ${HI}; do \
	cp  $D/src/include/$$i $$i;  \
	done
	for i in ${CC}; do \
	cp  $D/src/core/$$i $$i; \
	done
	mkdir -p bits
	for i in ${HB}; do \
	cp  $D/src/arch/i386/include/bits/$$i bits/$$i; \
	done
	for i in ${CDN}; do \
	cp  $D/src/drivers/net/$$i $$i; \
	done
	for i in ${CDD}; do \
	cp  $D/src/drivers/disk/$$i $$i; \
	done
	/bin/rm -rf $D
	patch -p0 < etherboot.patch
	mv -f bits/byteswap.h byteswap.h
	touch $@


etherboot.h:
	touch $@

timer.h:
	touch $@


clean:
	/bin/rm -rf $D  bits
	/bin/rm -f ${CDN} ${CDD} ${HB} ${CC} ${HI} ${ALL} done.stamp patch.stamp
	


