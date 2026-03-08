# SenecOS

A hobby x86-64 kernel written in C.

## Building

Read `gcc-cross-compiler.md` for build requirements.

```sh
make
make run
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
