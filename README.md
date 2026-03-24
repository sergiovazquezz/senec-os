# SenecOS

A hobby x86-64 kernel written in C.

## Building

Read `gcc-cross-compiler.md` for build requirements.

```bash
# No optimizations
make

# With optimizations
make release

# Remove the build
make clean
```

## Running

```bash
sudo pacman -S qemu-common qemu-system-x86 qemu-system-x86-firmware qemu-ui-gtk
```

```bash
# No optimizations
make run

# With GUI
make run-graphic

# With optimizations
make run-release

# With GDB
make debug
```

## Roadmap

- [x] Boot
- [x] VGA and Serial
- [x] GDT and TSS
- [x] IDT
- [x] ACPI parsing
- [x] Physical Memory Manager
- [ ] Virtual Memory
- [ ] APIC
- [ ] Timers
- [ ] Keyboard
- [ ] Scheduling
