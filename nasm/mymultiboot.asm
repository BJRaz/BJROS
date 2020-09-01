;org 0x100000				; addresses start origin - only enable if format is binary	
					; implemented by Brian Juul Rasmussen dec 2019/ jan 2020
bits 32					; forces nasm to generate a 32-bit image for 32-bit processor protected mode

extern kmain				; kmail is kernel main function ing if binary format.
global gdt:data
global idt_start:data			; start of interrupt descriptor table

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

	lgdt	[gdt]			; load global descriptor table register with 6 byte memory value
	lidt	[idt]			;

	push	eax			; contains magic value (magic number)
	push	ebx			; address of multiboot structure
	
 	call 	kmain			;_kmain			; call kernel main function

	mov	eax, cs			; move content of Code Section to eax (cs = hidden, visible)

	int	0x20

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
cursor:
.setcoords:
	; input bx = x, ax = y
	; this calculates to position as: y * VGA.W(80) + x	
	mov	dl, VGA.W		; (provided x = 5, y = 5) mov 80 (columns) to dl 
	mul	dl			; ax = al * dl -> ax = 5 * 80 -> 400 
	add	bx, ax			; bx = 5 + 400 -> 405 is the position (0000 0001 1001 0101)
.setoffset:
	; input bx = cursor offset
	mov	dx, 0x3d4		; select CRT index register
	mov	al, 0x0F
	out 	dx, al			; write 0x0f index to select cursor location low register

	inc	dl			; select CRT data register 0x3d5 
	mov	al, bl			; bl -> al
	out	dx, al			; write (1001 0101) to cursor location low register

	dec	dl			; select CRT index register 0x3d4
	mov	al, 0x0E		; 
	out	dx, al			; write 0x0e to select cursor location high register

	inc	dl			; select CRT data register
	mov	al, bh			; bh -> al
	out	dx, al			; write (0000 0001) to cursor location high register
	ret
	
disable_cursor:
	mov	dx, 0x3d4		; select CRT index register
	mov	al, 0x0a		; set index 
	out	dx, al			; disable cursor (bit 5 in cursor start register)
	
	inc	dx			; select CRT data register
	mov	al, 00100000b		; set data		 
	out	dx, al			; write data
	
	ret
print:
	mov	ecx, VGA_VIDEO
.loop:
	lodsb
	cmp	al, 0
	je	.exit
	mov	byte [ecx], al
	inc	ecx
	mov	byte [ecx], VIDEOATT
	inc	ecx
	jmp	.loop
.exit:
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
VIDEOTEXT		equ	0xff65ff65
VIDEOATT		equ	0x07
text			db	"Kernel loaded ..",0
text2			db	"Halted...", 0
VGA_VIDEO			equ	0xb8000
VGA.W			equ	80
; global descriptor table address and count - dont forget to align to 8 bytes boundary
align	2
gdt			dw	0x0010		; count of entries
			dd	0x00200000	; address
idt			dw	0x01ff		; count of entries
			dd	0x00202000	; address

section .bss
; this is the global descriptor table. The address is set via linker script (for section .gdt)
; can contain a maximem of 2^13 (8192) entries, and entry 0 is not used (called a null segment descriptor) 
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
idt_start:
			times 256	db 0
	
			dw	0x01dc		; offset B
			dw	0x0008		; segment 
			db	0x0
			db	10001111b
			dw	0x0011		; offset A			

			
