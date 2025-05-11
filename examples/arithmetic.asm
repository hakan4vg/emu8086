; Arithmetic Test Program for emu8086
; This file demonstrates arithmetic operations

    ; Basic register operations
    MOV AX, 5      ; Load 5 into AX
    MOV BX, 3      ; Load 3 into BX
    
    ; Addition
    ADD AX, BX     ; AX = AX + BX = 8
    
    ; Show result (convert to ASCII and display)
    ADD AX, 0x30   ; Convert to ASCII digit ('8' = 0x38)
    MOV AH, 0x0E   ; Teletype function
    INT 0x10       ; Display character '8'
    
    ; New line
    MOV AX, 0x0E0D ; CR
    INT 0x10
    MOV AX, 0x0E0A ; LF
    INT 0x10
    
    ; Comparison and jumping
    MOV CX, 5      ; Load 5 into CX
    MOV DX, 5      ; Load 5 into DX
    CMP CX, DX     ; Compare CX and DX
    JNE not_equal  ; Jump if not equal (which won't happen)
    
    ; If equal, print 'E'
    MOV AX, 0x0E45 ; AH=0E (teletype), AL='E'
    INT 0x10
    MOV AX, 0x0E71 ; 'q'
    INT 0x10
    MOV AX, 0x0E75 ; 'u'
    INT 0x10
    MOV AX, 0x0E61 ; 'a'
    INT 0x10
    MOV AX, 0x0E6C ; 'l'
    INT 0x10
    JMP end        ; Jump to end
    
not_equal:
    ; If not equal, print 'N'
    MOV AX, 0x0E4E ; AH=0E (teletype), AL='N'
    INT 0x10
    MOV AX, 0x0E6F ; 'o'
    INT 0x10
    MOV AX, 0x0E74 ; 't'
    INT 0x10
    
end:
    ; Halt the CPU
    HLT 