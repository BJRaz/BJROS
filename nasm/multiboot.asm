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
global gdt:data
global idt:data				; start of interrupt descriptor table
global kbdchar:data			; keybaord char data 
global kbdarray:data
global kbdarray_upper:data

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

	lgdt	[gdtr]			; load global descriptor table register with 6 byte memory value
	lidt	[idtr]			; load interrupt descriptor table register with 6 byte memory value
	
	jmp	0x8:setup
setup:
	mov	cx, 0x10
	mov	ds, cx
	mov	es, cx
	mov	fs, cx
	mov	gs, cx
	mov	ss, cx


	
	mov	esp, 0x7c00
	mov	ebp, esp 	
	
	push	eax			; contains magic value (magic number)
	push	ebx			; address of multiboot structure
	; TODO - needs to be done to initialize keyboard/mouse etc.
	; call	setup_ps2		; setup PS/2 controller (i8042)	
	call 	setup_pic		; init of PIC (programmable interrupt controller)
	;call 	setup_interrupts	; setup interrupt service routines etc..
	

	mov	word [cursor.y], 8	; initialization of variables
	mov	word [cursor.x], 0	; consider make them global

	sti				; enable interrupts
 	
	call 	kmain			;_kmain			; call kernel main function

	mov	eax, cs			; (test) stores visible content of Code Section to eax 
					; at this point the value should be 0d (00000000 00001000) - 1 = index 8
					; and 0 = GDT, 00 = priviledge level
mainloop:
	; TODO: busyloopw - uses 100% cpu .. fix somehow
;	jmp	mainloop
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
	mov	edi, VIDEO+VGA_COLS*2*(VGA_ROWS-1)	;
	mov	eax, 0 
	mov	ecx, VGA_BYTES_ROW
	rep 	stosb
	popad
	ret
; *******
;
; *******
setup_ps2:
	mov	al, 0x20
	out	0x64, al			; 0x20 read status 0 byte
		
	in	al, 0x64
	hlt

	ret
; *******
; PIC: setup programmable interrupt controller
; *******
setup_pic:
	; NOTICE:
	; at this point the PIC1 and PIC2 is NOT initialized properly. 
	; The PIC's interrupts needs remapping to other IRQ indexes/addresses:
	; ICWx -> Instruction Control Word, OCWx -> Operation Control Word

	mov	al, 00010001b		; IWC1: bit 1 => ICW4 is needed
	out	PIC1_CMD, al		; send ICW1 to PIC1 (master)
	out	PIC2_CMD, al		; send ICW1 to PIC2 (slave)

	mov	al, 00100000b		; ICW2: sets start addr for interrupt vectors 0-7
					; All vectors from 0-31 have been reserved by Intel, so set to 0x20 (32) 
	out	PIC1_DATA, al		; write the ICW2 to PIC1
	
	mov	al, 00101000b		; ICW2: sets start addr for interrupt vectors 8-15
					; Set to 0x28 (40) for slave
	out	PIC2_DATA, al		; write ICW2 to PIC2
	
	mov	al, 00000100b		; ICW3: set int line 2 for connecting PIC1 to PIC2
	out	PIC1_DATA, al		; write ICW3 to PIC1

	mov	al, 00000010b		; ICW3: set input line (from PIC1) to 2 in binary format (010 = 2)
					; for PIC2
	out	PIC2_DATA, al		; write ICW3 to pic2
	
	mov	al, 00000001b		; ICW4: bit 0 = 1 enables 80x86
	out	PIC1_DATA, al		; write to both controllers
	out	PIC2_DATA, al		;
				
	mov	al, 11111101b		; OCW1 interrupt mask = 11111101 
					; only IRQ1 (keyboard) is allowed trough
	out	PIC1_DATA, al		; write OCW1 to PIC1

	mov	al, 11111101b		; OCW1 interrupt mask = 11111101 
					; only IRQ12 (mouse) is allowed trough
	out	PIC2_DATA, al		; 
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
; ********
; IN (b, w, d)
; ********

global inb:function
inb:
	push	ebp		; stack management
	mov	ebp, esp
	push	edx		;
	mov	edx, [ebp+8]	; get argument / the register
	xor	eax, eax
	in	al, dx		; get byte from input register
	pop	edx		; restore edx
	
	mov	esp, ebp	; restore stack-frame
	pop	ebp
	ret
; ********
; OUT (b, w, d)
; ********

global outb:function
outb:
	push	ebp		; store basepointer
	mov	ebp, esp
	push	eax		; store value in eax on stack
	xor	eax, eax	
	mov	edx, [ebp+8]	; first argument (the register)
	mov	eax, [ebp+12]	; second argument (the value)
	out	dx, al		; write value to output register
	pop	eax		; restore eax from stack
	mov	esp, ebp	; restore stack frame
	pop	ebp
	ret
; *******
; ISR: interrupt service routines
; Notice: is contained within own section
; *******
section .isr
; ***************************
; division by zero handler
; ***************************
division_by_zero:
	;mov	esi,    text
	;call	print
	push	10
	push	text3
	call kprintf
	hlt				; at this point halts execution, due to the system continues 
					; calling the failing instruction (type fault)
	iret
; ***************************
; timer - handles the clockinterrupts
; ***************************
global timer:function
timer:
	pushad
	
	xor 	eax, eax
	xor 	ebx, ebx
	xor	edx, edx
	mov 	eax, [ticks]
	inc 	eax
	mov	[ticks], eax	
	mov	ebx, 0x02		; divide by 18 (18 times a second) 
					; TODO: this leeds to inaccurate timing as PIT ticks at 18.2 times a second
					; if used to represent time-ticks
	div	ebx			; qoutient = eax, remainder = edx .. explain
	cmp	edx, 0
	jne	.end
	mov	ebx, eax
	push	0x33
	call	_putchar
	pop	eax

;	push	word [ticks]
;	push	numbertxt
;	call	kprintf	
;	pop	eax
;	pop	ebx
.end:
	mov	al, 0x20		; EOI value
	out	PIC1_CMD, al		; send EOI to PIC1
;	pop	eax
	popad
	sti				; restore interrupts
	iret
; ****************************
; keyboard handler (i8042)
; note: 
; I/O port 0x64 (write) is send to the "onboard"-kbdcontroller 
; I/O port 0x60 (write) is send to the "in-kbd-case"-controller
; *---------*                *---------*
; *  0x64   *      <-->      *  0x60   *
; *---------*                *---------*
; (Motherboard)              (keyboard)
; ****************************
global keyboard:function
keyboard: 				
	cli
	pushad

	xor	eax, eax
.waitstatus:
	in	al, 0x64		; read status byte from i8042
	and	al, 0x1			; read byte 0, if al = 1, then buffer is full
	cmp	al, 0x1			; wait as long buffer is not full
	jne	.waitstatus


	in	al, 0x60		; read scancode from keyboard buffer (the keyboard encoder)
					; key press is called 'make', key release is called 'break'
	
	;cmp	al, 0x1c
	;jne	.cont
	;call	_scrollup
.cont:
;	push   	eax 				
;	push 	eax
	mov	ebx, eax		; store scancode - use stack ?
	and	al, 0x80		; check for bit 7 is 1 (the key is released - aka 'break')
	cmp	al, 0
	jnz	.break			; otherwize it is a 'make' - key pressed

					; here the key is pressed:
					; compare the scancode for identification of key 						
	cmp 	ebx, 0x2a		; L-shift
	je	.shift
	cmp	ebx, 0x36		; R-shift
	je	.shift
	jmp	.make
.shift:
	or 	byte [kbdstatus], 0x1		; set status byte to mark shift has been pressed
	jmp	.end	
.make:
	cmp     byte [kbdstatus], 0x1		; shift is pressed
	je	.makeshift
	mov	eax, [kbdarray+ebx]
	jmp	.makedone
.makeshift:
	mov	eax, [kbdarray_upper+ebx]
.makedone:
	mov	byte [kbdchar], al
.break:
	cmp	ebx, 0xAA		; L-shift released
	je	.shiftup
	cmp	ebx, 0xB6		; R-shift released
	je	.shiftup
	jmp	.end
.shiftup:
	and	byte [kbdstatus], 0	; 	
.end:
	mov	al, 0x20		; EOI value
	out	PIC1_CMD, al		; send EOI to PIC1
	popad
	sti		
	iret
; *********************
; custom (test) isr - declared global, and can be used in other modules
; *********************
global custom:function			
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
	out	PIC1_CMD, al		; send EOI
	pop	ebx	
	pop	eax	
	iret
; *******
; data section
; *******
section .data
	MB_HEADER_MAGIC		equ	0x1badb002
	MB_HEADER_FLAGS		equ	0x00000003
	MB_HEADER_CHECKSUM	equ	-(MB_HEADER_MAGIC+MB_HEADER_FLAGS)
	INT_DESCRIPTOR_OFFSETA	equ	0
	INT_DESCRIPTOR_SEGMENT	equ	2
	INT_DESCRIPTOR_FILL	equ	4
	INT_DESCRIPTOR_FLAGS	equ	5
	INT_DESCRIPTOR_OFFSETB	equ	6
	INT_DESCRIPTOR_SIZE	equ	8	

	kbdstatus	db	0	; internal keybaord status field
	
	text		db	"Kernel loaded ..",0
	text2		db	"Halted...", 0
	text3		db	"Division by zero!", 0xa, 0
	text4		db	"Keyboard IRS called %x", 0xa, 0
	numbertxt	db	"%d", 0xa, 0
	hextxt		db	"%x", 0xa, 0
	kbpressed	db	0
	kbdchar		db	0	; the last character from keyboard
	kbdarray	db	0, 0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', 0, 0x08
			db 	0x9,	; (f) tab
			db 	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 
			db	0x61,	; (1a) å
			db	'"',	; (1b) 
			db	0x0a, 	; CR (1c)
			db	0,	; CTRL (1d)
			db	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 
			db	0x61, 	; æ (27)
			db	0x6f, 	; ø (28)
			db	'$', 	; (29 unknown)
			db	0,	; shift left (2a)
			db	"'",	; (2b)
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
	kbdarray_upper	db	0, 0x1b, '!', '"', '#', 0, '%', '&', '/', '(', ')', '=', '?', '`', 0x08
			db 	0x9,	; (f) tab
			db 	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 
			db	0x61,	; (1a) Å
			db	'^',	; (1b)
			db	0x0a, 	; (1c) CR 
			db	0,	; (1d) CTRL 
			db	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 
			db	0x61, 	; (27) Æ
			db	0x6f, 	; (28) Ø
			db	'§', 	; (29)
			;db	0,	; (2a) shift left
			db	'*',	; (2b) 
			db	'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_',
			db	0, 	; (36) shift right 
			db	'*', 	
			db	0, 	; (38) alt left
			db	' ', 	; (39) space 
			db	0, 	; (3a) caps lock 
			db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0	; (3b-44) F1-F10 
			db	0, 	; (45)
			db	0,	; (46)
			db	'7', '8', '9', '-'		; (48 also arrow up)
			db	'4', '5', '6', '+'		; (4b, 4d) arrow left, arrow right
			db	'1', '2', '3', 			; (50) arrow down
			db	'0', ','
	ticks		dw	1			
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
; GDT: this is the global descriptor table setup. The address for section .gdt is set via linker script
; The gdt can contain a maximum of 2^13 (8192) entries, and entry 0 is not used (called a null segment descriptor) 
;  - dont forget to align to 8 bytes boundary 
; *******
section .gdt
gdt:
		db	11111100b	; first segment descriptor (not used)
		db	00000000b
		dw 	00000000b
		dw	00000000b
		dw	00000000b
					; CODE SEGMENT DESCRIPTOR	
		dd 	0x0000ffff	; base address (00-15), segment limit (0-15)
		db	00000000b	; base address (16-23)
		db 	10011011b	; type(1011 = Code,execute/read), 1001 => S=1, DPL=00, P=1
		db	11001111b	;
		db	00000000b
		
					; DATA SEGMENT DESCRIPTOR
		dd 	0x0000ffff	; base address (16-31), segment limit (0-15)
		db	00000000b
		db 	10010011b	; type(0011 = Data,execute/read), 1001 => S=1, DPL=00, P=1
		db	11001111b
		db	00000000b


; *******
; IDT: this is the interrupt descriptor table. The address for section .idt is set via linker script
; can contain a maximum of 256 entries
; *******
section .idt
idt:
		;dw	0x0034		; index 0, div by zero
		;dw	0x0008
		;db	0x0
		;db	10001110b
		;db	0x0011

		;times 256	db 0	; fill with 0 untill entry index 32
		
		; entry index 32(0x20):	
		;dw	0x01dc		; offset B
		;dw	0x0008		; segment (code segment) 
		;db	0x0		; fill bytes 
		;db	10001110b	; byte 8-12 0D110, byte 13-14 (DPL), 15 P (present flag)
		;dw	0x0011		; offset A			
		

