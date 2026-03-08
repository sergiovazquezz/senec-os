# GCC Cross Compiler

## 1. Install dependencies

```bash
sudo pacman -S base-devel gmp libmpc mpfr texinfo
```

## 2. Download from source

```bash
mkdir ~/cross-compiler
cd cross-compiler

# Ensure it's the latest version
wget https://ftp.gnu.org/gnu/binutils/binutils-2.46.0.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-15.2.0/gcc-15.2.0.tar.gz
wget https://ftp.gnu.org/gnu/gdb/gdb-17.1.tar.gz

tar xf binutils-2.46.0.tar.gz
tar xf gcc-15.2.0.tar.gz
tar xf gdb-17.1.tar.gz
```

## 3. Set environment variables

```bash
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"
```

## 4. Build binutils

```bash
mkdir ~/cross-compiler/build-binutils
cd build-binutils

../binutils-2.46.0/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror

# On macOS change nproc for: sysctl -n hw.logicalcpu
make -j$(nproc)

make install
```

## 5. Build GCC

```bash
mkdir ~/cross-compiler/build-gcc
cd build-gcc

../gcc-15.2.0/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers \
    --disable-hosted-libstdcxx

# On macOS change nproc for: sysctl -n hw.logicalcpu
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make -j$(nproc) all-target-libstdc++-v3

make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
```

## 6. Build GDB

```bash
mkdir ~/cross-compiler/build-gdb
cd build-gdb

../gdb-17.1/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror

# On macOS change nproc for: sysctl -n hw.logicalcpu
make -j$(nproc) all-gdb
make -j$(nproc) all-gdb
make install-gdb
```

## 7. Update .zshrc

```bash
export PATH="$HOME/opt/cross/bin:$PATH"
```
