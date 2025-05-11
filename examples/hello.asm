; Basic test program
; Using only instructions we know are supported

    ; Basic register operations
    MOV AX, 0x1234    ; Move immediate to register
    MOV BX, 0x5678    ; Move immediate to register
    ADD AX, BX        ; Add register to register
    
    ; Test jump instruction
    JMP end           ; Jump to end label
    
    ; This should be skipped
    HLT
    
end:
    ; End with halt instruction
    HLT 