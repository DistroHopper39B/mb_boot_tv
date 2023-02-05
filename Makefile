
ARCH=-arch i386

JC=main/eb.c main/eb-misc.c main/eb-nic.c main/eb-pci_bios.c \
	main/eb-pci.c main/eb-pci_drivers.c main/eb-pci_probe.c \
	main/eb-probe.c main/eb-rtl8139.c main/main.c \
	main/pci.c main/sleep.c main/time.c main/timer.c  \
	main/netboot.c main/string.c \
	main/eb-ide_disk.c main/eb-disk.c main/disk.c \
	main/hexdump.c main/diskboot.c  \
	main/partition.c main/efi.c main/e2.c main/malloc.c \
	main/crc.c main/version.c

JO=${JC:%.c=%.o}

OBJ=start.o loader_init.o vsprintf.o BootVideoHelpers.o sha1.o system.o ${JO}

P=i386-apple-darwin8-
S=-4.0

mb_boot_tv:  $(OBJ)
	${P}ld $(ARCH) -o mb_boot_tv $(OBJ) -image_base 0xB000000

%.o:	%.c
	${P}gcc${S} -c $(ARCH) -nostdlib -o $@ -c $<


%.o:	%.s
	${P}gcc${S} -c $(ARCH) -nostdlib -DASSEMBLER -o $@ -c $<

${JO}: main/eb/done.stamp

main/eb/done.stamp:
	${MAKE} -C main/eb done.stamp


#bzimage.h: bzImage gen_header
#	./gen_header
#	
#
#loader_init.o:bzimage.h
#gen_header: gen_header.c sha1.c
#	gcc -o gen_header gen_header.c sha1.c


clean:
	rm -f *.o mb_boot_tv gen_header check_header bzImage bzimage.h
	make -C main clean
	make -C main/eb clean
	rm -f version-num 

VFS=${shell cat version-files}
VCHK=${shell cat ${VFS} | md5sum | awk '{print $$1 }' }
VNUM=${shell grep ${VCHK} version-md5sums | awk '{ print $$2 }'  }
VDEF=${shell echo `cat version-major`.`cat version-minor`.`cat version-micro` }

main/version.o:main/version.h 

main/version.h: version-files version-major \
	version-minor version-micro \
	version-md5sums Makefile
	if [ .${VNUM} = . ]; then \
		echo "#define VERSION \"mb_boot_tv version ${VDEF} + Edits\"" > main/version.h; \
		echo ${VDEF}-E > version-num; \
	else \
		echo "#define VERSION \"mb_boot_tv version ${VNUM}\"" > main/version.h; \
		echo ${VNUM} > version-num; \
	fi

include main/depends
