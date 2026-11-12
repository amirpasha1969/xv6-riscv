FROM ubuntu:24.04
RUN apt update && apt install -y git build-essential \
    gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu \
    binutils-riscv64-linux-gnu