cmake_minimum_required(VERSION 3.5)
project(BJROS)
add_executable(kernel.elf kernel/kernel.c)

target_include_directories(kernel.elf PUBLIC multiboot include)
target_compile_options(kernel.elf PUBLIC -fuse-ld=i386-elf-ld -target i386-pc-none-elf -m32 -pedantic -c -nostdinc -ffreestanding)
