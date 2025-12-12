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

To be developed:
 
 - processes and interprocess communication 
 - memory management, and layout
 - I/O
  
