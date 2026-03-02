CC      := x86_64-elf-gcc
OBJCOPY := x86_64-elf-objcopy
GDB     := x86_64-elf-gdb

# GCC system header paths (for clangd)
GCC_INCLUDES       := $(shell $(CC) -print-file-name=include)
GCC_INCLUDES_FIXED := $(shell $(CC) -print-file-name=include-fixed)

COMMON_FLAGS := -ffreestanding -fno-builtin -nostdinc \
				-isystem $(GCC_INCLUDES) \
				-fno-stack-protector -mno-red-zone -mcmodel=kernel \
				-mno-sse -mno-mmx -mno-sse2 \
				-masm=intel \
				-Wall -Wextra -g -gdwarf-4
RELEASE_FLAGS := $(COMMON_FLAGS) -O2
DEBUG_FLAGS   := $(COMMON_FLAGS) -O0 -fanalyzer
CFLAGS  := $(DEBUG_FLAGS) -std=gnu17 -MMD -MP
ASFLAGS := $(DEBUG_FLAGS) -x assembler-with-cpp -MMD -MP
LDFLAGS := -T src/kernel.ld -nostdlib

BUILD_DIR := build
KERNEL_BIN := kernel.elf
ISO_DIR := iso
ISO_FILE := kernel.iso
LOGS_DIR := logs

C_SRCS := $(shell find src -name '*.c')
S_SRCS := $(shell find src -name '*.S')

C_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
S_OBJS := $(patsubst src/%.S,$(BUILD_DIR)/%.o,$(S_SRCS))
ALL_OBJS := $(S_OBJS) $(C_OBJS)

QEMU := qemu-system-x86_64
QEMU_KILL = ss -tlnp | grep :1235 | awk '{match($$0,/pid=([0-9]+)/,a); if(a[1]) print a[1]}' | xargs -r kill 2>/dev/null; true
QEMU_BASE := -drive file=kernel.iso,format=raw \
	-m 512M \
	-cpu host \
	-enable-kvm
QEMU_FLAGS := $(QEMU_BASE) \
	-serial stdio \
	-monitor telnet:127.0.0.1:1235,server,nowait
QEMU_FLAGS_NOGRAPHIC := $(QEMU_FLAGS) -nographic
QEMU_DEBUG_FLAGS := $(QEMU_BASE) \
	-nographic \
	-serial file:$(LOGS_DIR)/serial.log \
	-monitor telnet:127.0.0.1:1235,server,nowait \
	-s -S

all: $(ISO_FILE)

$(BUILD_DIR)/%.o: src/%.S
	@mkdir -p $(@D)
	@$(CC) $(ASFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_BIN): $(ALL_OBJS)
	@$(CC) $(LDFLAGS) -o $@ $^
	@echo "Kernel built: $@"

$(ISO_FILE): $(KERNEL_BIN)
	@echo "Creating bootable ISO..."
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp src/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)
	@echo "ISO created: $@"

clean:
	@rm -rf $(BUILD_DIR) $(KERNEL_BIN) $(ISO_FILE) $(ISO_DIR)/boot

run: $(ISO_FILE)
	$(QEMU) $(QEMU_FLAGS_NOGRAPHIC)

run-graphic: $(ISO_FILE)
	$(QEMU) $(QEMU_FLAGS)

run-log: $(ISO_FILE)
	mkdir -p $(LOGS_DIR)
	$(QEMU) $(QEMU_FLAGS_NOGRAPHIC) -d int,cpu_reset -D $(LOGS_DIR)/qemu.log

release: CFLAGS  := $(RELEASE_FLAGS) -std=gnu17 -MMD -MP
release: ASFLAGS := $(RELEASE_FLAGS) -x assembler-with-cpp -MMD -MP
release: $(ISO_FILE)
	@echo "Release build complete."

run-release: CFLAGS  := $(RELEASE_FLAGS) -std=gnu17 -MMD -MP
run-release: ASFLAGS := $(RELEASE_FLAGS) -x assembler-with-cpp -MMD -MP
run-release: $(ISO_FILE)
	$(QEMU) $(QEMU_FLAGS)

debug: $(ISO_FILE)
	@$(QEMU_KILL)
	mkdir -p $(LOGS_DIR)
	@echo "Serial output -> $(LOGS_DIR)/serial.log"
	$(QEMU) $(QEMU_DEBUG_FLAGS) &
	$(GDB) $(KERNEL_BIN) \
		-ex 'set tcp auto-retry on' \
		-ex 'set tcp connect-timeout 30' \
		-ex 'target remote :1234' \
		-ex 'hbreak kmain' \
		-ex 'continue'
	@$(QEMU_KILL)

dump: $(KERNEL_BIN)
	@echo "=== Section Layout ==="
	x86_64-elf-objdump -h $< | grep -v -i "debug"
	@echo ""
	@echo "=== Symbol Map ==="
	x86_64-elf-nm --numeric-sort --print-size $< | grep -v " [an] "

disasm: $(KERNEL_BIN)
	@echo "=== .text.boot (32-bit) ===" > kernel.objdump
	@x86_64-elf-objdump -d -m i386 -M intel -j .text.boot $< >> kernel.objdump
	@echo "" >> kernel.objdump
	@echo "=== .text (64-bit) ===" >> kernel.objdump
	@x86_64-elf-objdump -S -l -M intel -j .text $< >> kernel.objdump
	@echo "Full disassembly written to kernel.objdump"

.clangd: Makefile
	@printf 'CompileFlags:\n  Add:\n' > $@
	@printf '    - --target=x86_64-elf\n' >> $@
	@printf '    - -ffreestanding\n' >> $@
	@printf '    - -mno-red-zone\n' >> $@
	@printf '    - -isystem\n' >> $@
	@printf '    - $(GCC_INCLUDES)\n' >> $@
	@printf '    - -isystem\n' >> $@
	@printf '    - $(GCC_INCLUDES_FIXED)\n' >> $@

-include $(ALL_OBJS:.o=.d)

.PHONY: all clean run run-graphic run-release run-log release debug dump disasm
