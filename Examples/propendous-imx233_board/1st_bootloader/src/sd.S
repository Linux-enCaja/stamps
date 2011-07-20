
.code 32

.global sd_init, sd_next, sd_skip, sd_stop, sd_ctrl


sd_init: //(rom_BootInit_t *pInit) {
    push    {r7, lr}
//    ldr     r7, =0xffff7b78+1
    ldr     r7, =0xffff8208+1
    blx     r7
    pop     {r7, pc}


sd_next:    //(int *pCount) {
    push    {r4, lr}
//    ldr     r4, =0xffff7d80+1
    ldr     r4, =0xffff8410+1
    blx     r4
    pop     {r4, pc}


sd_skip:    //(int count) {
    push    {r4, lr}
//    ldr     r4, =0xffff7f40+1
    ldr     r4, =0xffff85d0+1
    blx     r4
    pop     {r4, pc}


sd_stop:    //(void) {
    push    {r4, lr}
//    ldr     r4, =0xffff7f58+1
    ldr     r4, =0xffff85e8+1
    blx     r4
    pop     {r4, pc}


sd_ctrl:    //(int action, void *pArg) {
    push    {r4, lr}
    ldr     r0, =test+1
    blx     r0
    b       end_of_code

.code 16
test:
    mov     r0, #42
    bx      r14

.code 32
end_of_code:
//    mov     r4, r0
    pop     {r4, pc}

