;org 0x100000				; addresses start origin - only enable if format is binary	
					; implemented by BJR dec 2019/ jan 2020
bits 32					; forces nasm to generate a 32-bit image for 32-bit processor protected mode

extern kmain				; kmail is kernel main function ing if binary format.

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
	push	eax			; contains magic value (magic number)
	push	ebx			; address of multiboot structure
	
 	call 	_kmain			;_kmain			; call kernel main function

	mov	eax, cs			; move content of Code Section to eax (cs = hidden, visible)

	hlt
_kmain:	; dummy kernel main function
	; do some work (test only)
	;mov 	dword [0xb8000],videotext ; writes text on screen	
	mov	eax, 16			
	;call 	disable_cursor
	mov	esi, text
	call 	print
	mov	ax, 5			; y 
	mov	bx, 5			; x
	call	cursor
	; do some out/in on vga mem to init cursor, and move cursor.
	ret
cursor:
.setcoords:
	; input bx=x, ax=y
	; this calculates to position as: y * vga.w(80) + x	
	mov	dl, vga.w		; (provided x = 5, y = 5) mov 80 to dl 
	mul	dl			; ax = al * dl -> ax = 5 * 80 -> 400 
	add	bx, ax			; bx = 5 + 400 -> 405 is the position (0000 0001 1001 0101)
.setoffset:
	; input bx = cursor offset
	mov	dx, 0x3d4
	mov	al, 0x0F
	out 	dx, al			; write 0x0f to select cursor location low register 0x0F

	inc	dl			; 0x3d5 (data)
	mov	al, bl			; bl -> al
	out	dx, al			; write (1001 0101) to cursor location low register

	dec	dl			; 0x3d4
	mov	al, 0x0E		; 
	out	dx, al			; write 0x0e to select cursor location high register

	inc	dl			; 0x3d5 (data)
	mov	al, bh			; bh -> al
	out	dx, al			; write (0000 0001) to cursor location high register
	ret
	
disable_cursor:
	mov	dx, 0x3d4		;
	mov	al, 0x0a		; 
	out	dx, al			; disable cursor (bit 5 in cursor start register)
	
	inc	dx
	mov	al, 00100000b		; 
	out	dx, al			;
	
	ret
print:
	mov	ecx, video
.loop:
	lodsb
	cmp	al, 0
	je	.exit
	mov	byte [ecx], al
	inc	ecx
	mov	byte [ecx], attribute
	inc	ecx
	jmp	.loop
.exit:
	ret

section .data
MB_HEADER_MAGIC		equ	0x1badb002
MB_HEADER_FLAGS		equ	0x00010003
MB_HEADER_CHECKSUM	equ	-(MB_HEADER_MAGIC+MB_HEADER_FLAGS)
videotext		equ	0xff65ff65
attribute		equ	0x07
text			db	"Kernel loaded ..",0
video			equ	0xb8000
vga.w			equ	80
section .bss

