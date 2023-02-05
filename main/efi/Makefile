SRCS=${wildcard *.c} 
OBJS=${SRCS:%.c=%.o}
CFLAGS=-g

all: efi

efi: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJS} 

protos:
	echo > prototypes.h
	cproto ${SRCS} > prototypes.h.new
	mv -f prototypes.h.new prototypes.h

clean:
	/bin/rm -f ${OBJS} efi

dep:
	makedepend ${SRCS}
# DO NOT DELETE

crc.o: project.h /usr/include/stdint.h /usr/include/features.h
crc.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
crc.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
crc.o: prototypes.h
disk.o: /usr/include/fcntl.h /usr/include/features.h /usr/include/sys/cdefs.h
disk.o: /usr/include/gnu/stubs.h /usr/include/bits/fcntl.h
disk.o: /usr/include/sys/types.h /usr/include/bits/types.h
disk.o: /usr/include/bits/wordsize.h
disk.o: /usr/lib/gcc/i386-redhat-linux/3.4.3/include/stddef.h
disk.o: /usr/include/bits/typesizes.h /usr/include/time.h
disk.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
disk.o: /usr/include/bits/confname.h project.h /usr/include/stdint.h
disk.o: /usr/include/bits/wchar.h partition.h prototypes.h
e2.o: project.h /usr/include/stdint.h /usr/include/features.h
e2.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e2.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
e2.o: prototypes.h e2/ext2fs.c e2/ext2fs.h e2/ext2_private.h e2/ext2_fs.h
e2.o: e2/ext2_fs_sb.h e2/ext2_fs_i.h
efi.o: project.h /usr/include/stdint.h /usr/include/features.h
efi.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
efi.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
efi.o: prototypes.h mbr.h efi_partition.h efi.h
main.o: project.h /usr/include/stdint.h /usr/include/features.h
main.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
main.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
main.o: prototypes.h
malloc.o: project.h /usr/include/stdint.h /usr/include/features.h
malloc.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
malloc.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
malloc.o: prototypes.h
mem.o: project.h /usr/include/stdint.h /usr/include/features.h
mem.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
mem.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
mem.o: prototypes.h
partition.o: project.h /usr/include/stdint.h /usr/include/features.h
partition.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
partition.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
partition.o: partition.h prototypes.h
string.o: project.h /usr/include/stdint.h /usr/include/features.h
string.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
string.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h partition.h
string.o: prototypes.h
