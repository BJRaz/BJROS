# BJROS

## preface

This is a experimental kernel for a 32 bit, x86 system platform. 

Some of the features are:

 - multiboot compliant
 - grub2 bootloader
 - kernel's file format is ELF
 - sets up a IDT, and GDT 
 - Initializes PS2 for keyboard and mouse
 - Initializes and configs programmable interrupt controller (PIC)
 - sets up interrupts for i.e mouse, keyboard, real timer clock 
 - starts up in text mode, and have very basic input prompt.
 - Flat memory model - no segmentation
 - Microkernel-inspired design with round-robin preemptive scheduler
 - Console runs as a schedulable process (PID 1)
 - Ring buffers for keyboard input (ISR → process)
 - Heap allocator (`_malloc`/`_free`) at 0x300000 (1 MB)
 - Serial console output (COM1) for headless/QEMU boot

To be developed:
 
 - interprocess communication 
 - more processes and process management
 - I/O subsystem
  
