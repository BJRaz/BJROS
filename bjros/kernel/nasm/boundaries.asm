section .text
	mov	byte [mybyte], 255	
	mov	word [myword], 255
	mov	long [mylong], 255


section .data
i_gate_offset:	dw	0ah
i_gate_segment:	dw	0ah
i_gate_b1:	db	0
i_gate_b2:	db	00001110b
i_gate_offset2:	dw	0ffffh
; normalt ikke lagt i .data, assembler giver en warning
mybyte:	db	1
	alignb	2
myword:	dw	1
	alignb	4
mylong:	dd	1
mystr:	db	2
	align	8	
end:	db	"E"
