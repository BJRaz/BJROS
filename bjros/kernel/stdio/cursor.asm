; cursor routines 
; borrowed from OSDev Wiki
; Brian J Rasmussen, aug 2020 
section .text
global	cursor:function
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
global  print:function
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
section .data
VGA_VIDEO		equ	0xb8000
VGA.W			equ	80
VIDEOTEXT		equ	0xff65ff65
VIDEOATT		equ	0x06

