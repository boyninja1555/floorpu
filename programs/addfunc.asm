; r0 = return value
; r1 = post-addition jump address LOW
; r2 = post-addition jump address HIGH
; r3 = left addition operand
; r4 = right addition operand

_start0:
    pusha _start1  ; Push A: post-addition jump address
    pushl 1        ; Push B: left addition operand
    pushl 1        ; Push C: right addition operand
    jump add       ; Calls addition function

_start1:
    set r0 0
    set r1 0
    set r2 0
    set r3 0
    set r4 0

    pop r0  ; Pop D: return value
    halt

add:
    pop r4        ; Pop C: right addition operand
    pop r3        ; Pop B: left addition operand
    popa r1 r2    ; Pop A: post-addition jump address
    add r0 r3 r4  ; Does addition
    push r0       ; Push D: return value
    jumpr r1 r2   ; Returns from addition function
