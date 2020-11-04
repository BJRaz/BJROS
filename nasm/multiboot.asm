;org 0x100000				; addresses start origin - only enable if format is binary	
					; implemented by Brian Juul Rasmussen dec 2019/ jan 2020
; MULTIBOOT comliant kernel
; state after bootloader is:
; - eax: must contain the magic value '0x2BADB002'
; - ebx: must contain the psysical address of multiboot information structure (provided by bootloader)
; - cs: 32 bit read/execute secment with offset of '0', and limit of '0xFFFFFFFF'. Exact value is undefined
; - ds,es,fs,gs,ss: as mentioned in 'cs'
; - a20 gate: enabled
; - cr0: bit 31 (pg) must be cleared, bit 0 (PE) must be set, other bits are undefined.
; - eflags: bit 17 (VM) must be cleared, bit 9 (IF) must be cleared. Other bits undefined.
; - esp: undefined
; - gdtr, ldtr: undefined.

bits 32					; forces nasm to generate a 32-bit image for 32-bit processor protected mode
; *******
; interrupt definitions:
; *******
%define	DIV_ZERO	0x0
%define TIMER		0x8
%define KEYB		0x9
%define CUSTOM		0x20

%define PIC1_DATA	0x21		; master programmable interrupt controller data port
%define PIC1_CMD	0x20		;  - command port	
%define PIC2_DATA	0xa1
%define PIC2_CMD	0xa0

%define STACK_SIZE	0x4000
%define	VIDEO		0xb8000		; VIDEO address for text color mode
%define VGA_ROWS	25
%define	VGA_COLS	80
%define VGA_BYTES_ROW	160		; in vga text mode 3 - 80x25 16 colors uses 2 bytes mem pr character
extern kmain				; kmail is kernel main function ing if binary format.
extern cursor
extern print
extern kprintf
extern _putchar

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

	cli
	
	mov	esp, 0x7c00
	mov	ebp, esp 	
	
	push	eax			; contains magic value (magic number)
	push	ebx			; address of multiboot structure
	
	call 	setup_pic		; init of PIC (programmable interrupt controller)
	;call 	setup_interrupts	; setup interrupt service routines etc..
	
	lgdt	[gdtr]			; load global descriptor table register with 6 byte memory value
	lidt	[idtr]			; load interrupt descriptor table register with 6 byte memory value

	mov	word [cursor.y], 8	; initialization of variables
	mov	word [cursor.x], 0	; consider make them global

	sti				; enable interrupts

 	call 	kmain			;_kmain			; call kernel main function

	mov	eax, cs			; (test) stores visible content of Code Section to eax 
					; at this point the value should be 0d (00000000 00001000) - 1 = index 8
					; and 0 = GDT, 00 = priviledge level
mainloop:
	; TODO: busyloopw - uses 100% cpu .. fix somehow
	jmp	mainloop
;	int	0x20
	hlt
; *******
; VGA
; *******
global _scrollup:function
_scrollup:	
	pushad
	mov	esi, VIDEO+VGA_BYTES_ROW
	mov	edi, VIDEO
	mov	ecx, VGA_BYTES_ROW*VGA_ROWS		; count 160x25 bytes
	rep	movsb	
	; clear last line
	mov	edi, VIDEO+VGA_COLS*2*(VGA_ROWS-1)		;
	mov	eax, 0 
	mov	ecx, VGA_BYTES_ROW
	rep 	stosb
	popad
	ret
; *******
; PIC: setup programmable interrupt controller
; *******
setup_pic:
	; NOTICE:
	; at this point the PIC1 and PIC2 is NOT initialized properly. 
	; The PIC's interrupts needs remapping to other IRQ indexes/addresses.
	mov	al, 0xfd		; OCW1 interrupt mask = 11111101 
					; only IRQ1 (keyboard) is allowed trough
					; TODO: remember to activate timer etc later.
	out	PIC1_DATA, al		; write OCW1 to PIC1
	ret
; *******
; IDT: setup interrupt handlers in IDT
; *******
setup_interrupts:	
	lea	eax, [division_by_zero]
	mov	word [idt+INT_DESCRIPTOR_OFFSETA+(INT_DESCRIPTOR_SIZE*DIV_ZERO)], ax
	mov	word [idt+INT_DESCRIPTOR_SEGMENT+(INT_DESCRIPTOR_SIZE*DIV_ZERO)], 0x8
	mov	byte [idt+INT_DESCRIPTOR_FILL+(INT_DESCRIPTOR_SIZE*DIV_ZERO)], 0
	mov	byte [idt+INT_DESCRIPTOR_FLAGS+(INT_DESCRIPTOR_SIZE*DIV_ZERO)], 10001110b
	rol	eax, 16
	mov	word [idt+INT_DESCRIPTOR_OFFSETB+(INT_DESCRIPTOR_SIZE*DIV_ZERO)], ax
	
	lea	eax, [timer]
	mov	word [idt+INT_DESCRIPTOR_OFFSETA+(INT_DESCRIPTOR_SIZE*TIMER)], ax
	mov	word [idt+INT_DESCRIPTOR_SEGMENT+(INT_DESCRIPTOR_SIZE*TIMER)], 0x8
	mov	byte [idt+INT_DESCRIPTOR_FILL+(INT_DESCRIPTOR_SIZE*TIMER)], 0
	mov	byte [idt+INT_DESCRIPTOR_FLAGS+(INT_DESCRIPTOR_SIZE*TIMER)], 10001110b
	rol	eax, 16
	mov	word [idt+INT_DESCRIPTOR_OFFSETB+(INT_DESCRIPTOR_SIZE*TIMER)], ax

	;lea	eax, [keyboard]
	;mov	word [idt+INT_DESCRIPTOR_OFFSETA+(INT_DESCRIPTOR_SIZE*KEYB)], ax
	;mov	word [idt+INT_DESCRIPTOR_SEGMENT+(INT_DESCRIPTOR_SIZE*KEYB)], 0x8
	;mov	byte [idt+INT_DESCRIPTOR_FILL+(INT_DESCRIPTOR_SIZE*KEYB)], 0
	;mov	byte [idt+INT_DESCRIPTOR_FLAGS+(INT_DESCRIPTOR_SIZE*KEYB)], 10001110b
	;rol	eax, 16
	;mov	word [idt+INT_DESCRIPTOR_OFFSETB+(INT_DESCRIPTOR_SIZE*KEYB)], ax
	
	lea	eax, [custom]
	mov	word [idt+INT_DESCRIPTOR_OFFSETA+(INT_DESCRIPTOR_SIZE*CUSTOM)], ax
	mov	word [idt+INT_DESCRIPTOR_SEGMENT+(INT_DESCRIPTOR_SIZE*CUSTOM)], 0x8
	mov	byte [idt+INT_DESCRIPTOR_FILL+(INT_DESCRIPTOR_SIZE*CUSTOM)], 0
	mov	byte [idt+INT_DESCRIPTOR_FLAGS+(INT_DESCRIPTOR_SIZE*CUSTOM)], 10001110b
	rol	eax, 16
	mov	word [idt+INT_DESCRIPTOR_OFFSETB+(INT_DESCRIPTOR_SIZE*CUSTOM)], ax
	
	ret
; *******
; dummy kernel main function
; *******
_kmain:
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
; *******
; ISR: interrupt service routines
; Notice: is contained within own section
; *******
section .isr
division_by_zero:
	;mov	esi,    text
	;call	print
	push	10
	push	text3
	call kprintf
	hlt				; at this point halts execution, due to the system continues 
					; calling the failing instruction (type fault)
	iret
timer:
	mov	al, 0x20		; EOI value
	out	PIC1_CMD, al		; send EOI to PIC1
	iret
global keyboard:function
keyboard: 				; keyboard handler
	pushad
	xor	eax, eax
	in	al, 0x60		; read scancode from keyboard buffer
	;cmp	al, 0x1c
	;jne	.cont
	;call	_scrollup
.cont:
;	push   	eax 				
;	push 	eax
	mov	ebx, eax		; store scancode
	and	al, 0x80		; check for bit 7 is 1 (the key is released - aka 'break')
	cmp	al, 0
	jnz	.break
.make:					; otherwize it is a 'make' - key pressed
	;push	ebx			; arg1 - push the scancode
	;push	text4			; arg0 - push the format-string
	;call	kprintf			; print the scancode
	mov	byte ebx, [kbdarray+ebx]
	push	ebx	
	call 	_putchar

	pop	ebx			; reset stack
	;pop	eax
.break:
	mov	al, 0x20		; EOI value
	out	PIC1_CMD, al		; send EOI to PIC1
	popad
	sti		
	iret
global custom:function			; custom (test) isr - declared global, and can be used in other modules
custom: 
	push	eax
	push	ebx
	xor	eax, eax
	mov	ax, [cursor.y]
	mov	bx, [cursor.x]
	call	cursor
	inc	ax
	inc	bx
	mov	[cursor.y], ax
	mov	[cursor.x], bx
	mov	al, 0x20
	out	0x20, al		; send EOI
	pop	ebx	
	pop	eax	
	iret
; *******
; data section
; *******
section .data
	MB_HEADER_MAGIC		equ	0x1badb002
	MB_HEADER_FLAGS		equ	0x00010003
	MB_HEADER_CHECKSUM	equ	-(MB_HEADER_MAGIC+MB_HEADER_FLAGS)
	INT_DESCRIPTOR_OFFSETA	equ	0
	INT_DESCRIPTOR_SEGMENT	equ	2
	INT_DESCRIPTOR_FILL	equ	4
	INT_DESCRIPTOR_FLAGS	equ	5
	INT_DESCRIPTOR_OFFSETB	equ	6
	INT_DESCRIPTOR_SIZE	equ	8	
	
	text		db	"Kernel loaded ..",0
	text2		db	"Halted...", 0
	text3		db	"Division by zero!", 0xa, 0
	text4		db	"Keyboard IRS called %x", 0xa, 0
	numbertxt	db	"%d", 0xa, 0
	hextxt		db	"%x", 0xa, 0
	kbpressed	db	0
	kbdarray	db	0, 0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', 0, 0x08
			db 	0x9,	; tab
			db 	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 
			db	0x61,	; å
			db	'"',
			db	0x0a, 	; CR (1c)
			db	0,	; CTRL (1d)
			db	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 
			db	0x61, 	; æ (27)
			db	0x6f, 	; ø (28)
			db	'$', 	; (29 unknown)
			db	0,	; shift left (2a)
			db	0,	; (2b)
			db	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-',
			db	0, 	; shift right (36)
			db	'*', 	
			db	0, 	; alt left (38)
			db	' ', 	; space (39)
			db	0, 	; caps lock (3a)
			db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0	; F1-F10 (3b-44)
			db	0, 	; (45)
			db	0,	; (46)
			db	'7', '8', '9', '-'		; (48 also arrow up)
			db	'4', '5', '6', '+'		; (4b, 4d) arrow left, arrow right
			db	'1', '2', '3', 			; (50) arrow down
			db	'0', ','
				
;				
				 	
; *******
; GDTR: global descriptor register table setup 
; *******
align	2
gdtr		dw	0x01ff		; count of entries
		dd	0x00200000	; address
idtr		dw	0x01ff		; count of entries
		dd	0x00202000	; address
align	4

; *******
; BSS section 
; *******
section .bss
	cursor.x	resb	32	
	cursor.y	resb	32


; ******
; GDT: this is the global descriptor table. The address is set via linker script (for section .gdt)
;  can contain a maximem of 2^13 (8192) entries, and entry 0 is not used (called a null segment descriptor) 
;  - dont forget to align to 8 bytes boundary (Why ?)
; *******
section .gdt
		db	11111100b	; first segment descriptor (not used)
		db	00000000b
		dw 	00000000b
		dw	00000000b
		dw	00000000b

		dd 	0x0000ffff	; base address (16-31), segment limit (0-15)
		db	00000000b
		db 	10011011b	; type(1011 = Code,execute/read), S=1, DPL=001, P=1
		db	11001111b
		db	00000000b
		
		dd 	0x0000ffff	; base address (16-31), segment limit (0-15)
		db	00000000b
		db 	10010011b	; type(0011 = Data,execute/read), S=1, DPL=001, P=1
		db	11001111b
		db	00000000b


		;dd	0x00cf9b00	; base adderss, type etc, segment limit, base ...
; *******
; IDT: this is the interrupt descriptor table. The address is set via linker script (for section .idt)
; can contain a maximum of 256 entries
; *******
section .idt
idt:
		;dw	0x0034		; index 0, div by zero
		;dw	0x0008
		;db	0x0
		;db	10001110b
		;db	0x0011

		times 256	db 0	; fill with 0 untill entry index 32
		; entry index 32(0x20):	
		dw	0x01dc		; offset B
		dw	0x0008		; segment 
		db	0x0		; fill bytes 
		db	10001110b	; byte 8-12 0D110, byte 13-14 (DPL), 15 P (present flag)
		dw	0x0011		; offset A			
		

