	.file	"itoa.c"
	.section	.rodata
.LC0:
	.string	"Number: %d\n"
.LC1:
	.string	"Buffer: %s\n"
.LC2:
	.string	"Very long: %llu\n"
	.text
.globl main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$80, %esp
	movl	$0, 76(%esp)
	cmpl	$1, 8(%ebp)
	jle	.L2
	movl	12(%ebp), %eax
	addl	$4, %eax
	movl	(%eax), %eax
	movl	%eax, 76(%esp)
.L2:
	movl	76(%esp), %eax
	movl	%eax, (%esp)
	call	_atoi
	movl	%eax, 72(%esp)
	movl	$.LC0, %eax
	movl	72(%esp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	printf
	leal	28(%esp), %eax
	movl	%eax, 4(%esp)
	movl	72(%esp), %eax
	movl	%eax, (%esp)
	call	_itoa
	movl	%eax, 68(%esp)
	movl	$.LC1, %eax
	leal	28(%esp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	printf
	movl	$10, 56(%esp)
	movl	$0, 60(%esp)
	movl	$20, 48(%esp)
	movl	$0, 52(%esp)
	movl	48(%esp), %eax
	movl	52(%esp), %edx
	addl	%eax, 56(%esp)
	adcl	%edx, 60(%esp)
	movl	$.LC2, %ecx
	movl	56(%esp), %eax
	movl	60(%esp), %edx
	movl	%eax, 4(%esp)
	movl	%edx, 8(%esp)
	movl	%ecx, (%esp)
	call	printf
	movl	68(%esp), %eax
	leave
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 4.5.1 20100924 (Red Hat 4.5.1-4)"
	.section	.note.GNU-stack,"",@progbits
