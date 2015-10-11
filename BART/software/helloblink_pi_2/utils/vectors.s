/* 
 * first.s
 *
 * This is a verry simple assemble program. To compile it follow these
 * two steps:
 *
 * $ arm-linux-as vectors.s -o vectors.o
 * 
 */

;@-------------------------------------------------------------------------
;@-------------------------------------------------------------------------

.globl PUT32
PUT32:
    str r1,[r0]
    bx lr


.globl GET32
GET32:
    ldr r0,[r0]
    bx lr
