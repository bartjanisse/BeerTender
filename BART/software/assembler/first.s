/* 
 * first.s
 *
 * This is a verry simple assemble program. To compile it follow these
 * two steps:
 *
 * $ arm-linux-as first.s -o first.s
 * $ arm-linux-gcc first.s -o first
 *
 * Now you have an executable first. 
 * 
 */
 
.global main		/* main is our entry point and must be global */


main:				/* This is main */
	mov r0, #2		/* Put a 2 inside the register r0 */
	bx lr			/* Return from main */
