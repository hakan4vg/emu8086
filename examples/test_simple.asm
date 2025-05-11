; Simple test program
; Tests only a few instructions to help debug

    ; Basic register operations
    MOV AX, 5       ; Load 5 into AX
    MOV BX, 3       ; Load 3 into BX
    
    ; Addition
    ADD AX, BX      ; AX = 5 + 3 = 8
    
    ; Output success
    MOV AH, 0x0E    ; Teletype output function
    MOV AL, 'P'     ; 'P' for PASS
    INT 0x10        ; Call BIOS video service
    HLT             ; Halt the CPU 