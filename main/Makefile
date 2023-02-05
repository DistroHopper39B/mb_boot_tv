PSRCS=crc.c disk.c diskboot.c e2.c eb.c efi.c hexdump.c main.c malloc.c netboot.c partition.c pci.c sleep.c string.c time.c timer.c version.c


default:
	${MAKE} -C ..

clean:
	/bin/rm -f *.o *~ core a.out prototypes.h.tmp depends.bak *~ version.h


protos:
	echo > prototypes.h
	touch version.h
	cproto -v -e ${PSRCS} > prototypes.h.tmp
	mv -f prototypes.h.tmp prototypes.h
	rm -f version.h

depend:
	touch depends
	cd ..; makedepend -f main/depends ${PSRCS:%=main/%}

# DO NOT DELETE

