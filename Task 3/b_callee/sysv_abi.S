relativeTimeSince:
.global 	relativeTimeSince

    // TODO: Implement this function in x86 64-bit assembly using the System V AMD64 ABI
    //       See the wiki for the functionality you need to implement
    //       Note: 
    //          * time_t is of the type ssize_t (long int) on x86_64 
    //          * read the IDIV instruction description very carefully if you choose to use it 
    //            (Note the register layout for the input! The notation RDX:RAX defines the 
    //             higher 64 bit of a 128-bit word to be stored in rdx and the lower 64 bit in rax.)
    //          * this layout has further implication if the data ha a negative sign (hint: you may 
    //            need to sign-extend the register, there are instructions for this purpose!)

	pushq	%rbp
	movq	%rsp, %rbp
	subq	$40, %rsp
	movq	%rdi, -16(%rbp)
	movq	%rsi, -24(%rbp)
	movq	%rdx, -32(%rbp)
	movq	$0, %rdi
	movq	$201, %rax
	syscall
	movq	-16(%rbp), %rdx
	subq	%rdx, %rax
	movq 	%rax, -8(%rbp)
	movq	$31536000, %rcx
	movq	$0, %rdx
	cmpq	%rax, %rdx
	jns	.makeNeg
	divq  %rcx
	movq	%rax, %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, (%rax)

.continueNormaly1:
	movq	$86400, %rcx
	movq 	-8(%rbp), %rax
	movq	$0, %rdx
	cmpq	%rax, %rdx
	jns	.makeNeg2
	divq  %rcx
	movq	%rax, %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)

.continueNormaly2:
	movq	$-365, %rcx
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	imulq	%rcx
	movq	-32(%rbp), %rcx
	movq	(%rcx), %rdx
	addq	%rax, %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)

	movq 	-8(%rbp), %rax
	addq	$40, %rsp
	movq	%rbp, %rsp
	popq %rbp
  ret

.makeNeg:
	neg %rax
	divq  %rcx
	neg %rax
	movq	%rax, %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, (%rax)
	jmp	.continueNormaly1

.makeNeg2:
	neg %rax
	divq  %rcx
	neg %rax
	movq	%rax, %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, (%rax)	
	jmp	.continueNormaly2