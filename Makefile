CC = i686-elf-gcc
NASM = nasm

C_SOURCES = $(wildcard kernel/*.c)
HEADERS = $(wildcard kernel/include/*.h)
OBJ = ${C_SOURCES:.c=.o}

all: target/os-image

run: all
	qemu-system-i386 -drive format=raw,file=target/os-image 

# Build the disk image that the computer loads.
# This is a combination of the compiled boot sector, kernel, and padding to make the disk image a desired size.
target/os-image: boot/boot_sect.bin kernel/kernel.bin boot/pad.bin
	cat $^ > $@

# Kernel binary
# - ffrestanding implies no standard library exists (only a subset of C library with #define and typedefs only).
#   This is important as the compiler is building a kernel rather than user-space program. 
# - nostdlib removes some start files that should only be for user space programs.
# - lgcc expands to the full path of libgcc that only the compiler knows about. It was originally excluded by nostdlib.
kernel/kernel.bin: kernel/kernel_entry.o ${OBJ}
	$(CC) -T linker.ld -ffreestanding -Wall -O0 -nostdlib $^ -o $@ -lgcc

%.o: %.c ${HEADERS}
	$(CC) -ffreestanding -Wall -O0 -c $< -o $@ -lgcc

kernel/kernel_entry.o: kernel/kernel_entry.asm
	${NASM} $< -f elf -o $@

# Boot sector 
%.bin: %.asm
	${NASM} $< -f bin -i 'boot' -o $@

clean:
	rm -fr target/* kernel/*.bin kernel/*.o boot/*.bin
