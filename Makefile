CC = i686-elf-gcc
CFLAGS = -ffreestanding -Wall -O0 -nostdlib
NASM = nasm

BUILD_DIR = build
SRC_DIR = kernel
BOOT_DIR = boot
INCLUDE_DIR = kernel/include

C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.asm)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

OBJ = $(addprefix $(BUILD_DIR)/, $(notdir ${ASM_SOURCES:.asm=.o} ${C_SOURCES:.c=.o}))

all: $(BUILD_DIR)/os-image

run: all
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/os-image

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the disk image that the computer loads. This is a combination of the compiled boot sector, kernel, and padding to make the disk image a desired size.
$(BUILD_DIR)/os-image: $(BUILD_DIR)/boot_sect.bin $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/pad.bin
	cat $^ > $@

# Kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel_entry.o ${OBJ}
	$(CC) -T linker.ld $(CFLAGS) $^ -o $@ -lgcc

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ${HEADERS} | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -lgcc

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	${NASM} $< -f elf -o $@

$(BUILD_DIR)/%.bin: $(BOOT_DIR)/%.asm | $(BUILD_DIR)
	${NASM} $< -f bin -i '$(BOOT_DIR)' -o $@

clean:
	rm -fr $(BUILD_DIR)
