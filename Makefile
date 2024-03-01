C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
OBJ = ${C_SOURCES:.c=.o}

# Default build target
all: target/os-image

# Run qemu to simulate booting of the os disk image
run: all
	qemu-system-i386 -drive format=raw,file=target/os-image 

# Build the disk image that the computer loads.
# This is a combination of the compiled boot sector, kernel, and padding to make the disk image a desired size.
target/os-image: boot/boot_sect.bin kernel/kernel.bin boot/pad.bin
	cat $^ > $@

# Build the binary of the kernel by linking:
# 1) the kernel entry, which jumps to _start in the kernel
# 2) the compiled kernel code  
# - -Ttext 0x1000 offsets label addresses in kernel code to absolute memory addresses (kernel code is loaded at 0x1000 in physcial memory)
# - -oformat binary produces a raw binary format
kernel/kernel.bin: kernel/kernel_entry.o ${OBJ}
	i686-elf-ld -Ttext 0x1000 --oformat binary $^ -o $@

# Cross-compile kernel code to elf object files for i386 architecture (32-bit)
# - -ffrestanding implies no standard library exists (only a subset of C library with #define and typedefs only).
#  	This is important as it lets the compiler know it is building a kernel rather than user-space program. 
#  	The documentation for GCC says you are required to implement the functions memset, memcpy, memcmp and memmove yourself in freestanding mode.
# - -lgcc includes libgcc, which implements various runtime routines that the cross-compiler depends on.
%.o: %.c ${HEADERS}
	i686-elf-gcc -ffreestanding -Wall -O -c $< -o $@ -lgcc

# Assemble the kernel entry 
# - -f elf produces elf object file
%.o: %.asm
	nasm $< -f elf -o $@

# Build boot sector binary
# - -f bin produces raw binary
# - -I specifies includes directory
%.bin: %.asm
	nasm $< -f bin -I './boot/' -o $@

clean:
	rm -fr *.bin *.o os-image
	rm -fr kernel/*.o boot/*.bin drivers/*.o
