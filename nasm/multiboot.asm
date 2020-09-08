;org 0x100000				; addresses start origin - only enable if format is binary	
					; implemented by Brian Juul Rasmussen dec 2019/ jan 2020
bits 32					; forces nasm to generate a 32-bit image for 32-bit processor protected mode

extern kmain				; kmail is kernel main function ing if binary format.
extern cursor
global gdtr:data
global idt:data				; start of interrupt descriptor table

section .text
global _start
_start:
	jmp	multiboot_entry
	align 4				; 32 bit aligned	
multiboot_header:
	dd	MB_HEADER_MAGIC		; magic number
	dd	MB_HEADER_FLAGS		; flags
	dd	MB_HEADER_CHECKSUM
	dd	multiboot_header	; header address
	dd	_start			; load start of .text secment
	dd	0			; load end addr.
	dd	0			; bss end addr.
	dd	multiboot_entry		; entry addr.
multiboot_entry:

	lgdt	[gdtr]			; load global descriptor table register with 6 byte memory value
	lidt	[idtr]			;

	push	eax			; contains magic value (magic number)
	push	ebx			; address of multiboot structure
	
 	call 	kmain			;_kmain			; call kernel main function

	mov	eax, cs			; (test) stores visible content of Code Section to eax 
					; at this point the value should be 0d (00000000 00001000) - 1 = index 8
					; and 0 = GDT, 00 = priviledge level

	;int	0x20
	int	0x20			; interrupt index 32
	hlt



_kmain:	; dummy kernel main function
	; do some work (test only)
	;mov 	dword [0xb8000],VIDEOTEXT ; writes text on screen	
	mov	eax, 16			
	;call 	disable_cursor
	;mov	esi, text
	;call 	print
	mov	ax, 0			; y 
	mov	bx, 80			; x
	call	cursor
	; do some out/in on vga mem to init cursor, and move cursor.
	ret
; interrupt service routines:
section .isr
divition_by_zero:
	mov		eax, 0x1
	mov		gs, eax
	iret




section .data
MB_HEADER_MAGIC		equ	0x1badb002
MB_HEADER_FLAGS		equ	0x00010003
MB_HEADER_CHECKSUM	equ	-(MB_HEADER_MAGIC+MB_HEADER_FLAGS)
text			db	"Kernel loaded ..",0
text2			db	"Halted...", 0
; global descriptor table address and count 
align	2
gdtr			dw	0x0010		; count of entries
			dd	0x00200000	; address
idtr			dw	0x01ff		; count of entries
			dd	0x00202000	; address

section .bss
; this is the global descriptor table. The address is set via linker script (for section .gdt)
; can contain a maximem of 2^13 (8192) entries, and entry 0 is not used (called a null segment descriptor) 
; - dont forget to align to 8 bytes boundary
section .gdt
			db	11111100b	; first segment descriptor (not used)
			db	00000000b
			dw 	00000000b
			dw	00000000b
			dw	00000000b

			dd 	0x0000ffff	; base address (16-31), segment limit (0-15)
			dd	0x00cf9b00	; base adderss, type etc, segment limit, base ...
; this is the interrupt descriptor table. The address is set via linker script (for section .idt)
; can contain a maximum of 256 entries
section .idt
idt:
			times 256	db 0	; fill with 0 untill entry index 32
			; entry index 32(0x20):	
			dw	0x01dc		; offset B
			dw	0x0008		; segment 
			db	0x0		; fill bytes 
			db	10001110b	; byte 8-12 0D110, byte 13-14 (DPL), 15 P (present flag)
			dw	0x0011		; offset A			

			
