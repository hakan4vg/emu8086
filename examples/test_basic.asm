; Basic test program for CPU operations
; Tests only numeric operations

    ; Set up video mode
    MOV AX, 0x0003  ; AH=0 (set video mode), AL=3 (80x25 text)
    INT 0x10
    
    ; Test basic arithmetic
    MOV AX, 5       ; Load 5 into AX
    MOV BX, 3       ; Load 3 into BX
    
    ; Addition and subtraction
    ADD AX, BX      ; AX = 5 + 3 = 8
    SUB AX, 1       ; AX = 8 - 1 = 7
    
    ; Comparison and flag operations
    CMP AX, 7       ; Compare AX with 7 (should set ZF)
    CLC             ; Clear carry flag
    STC             ; Set carry flag
    CMC             ; Complement carry flag
    
    ; Output success code
    MOV AH, 0x0E    ; Teletype output function
    MOV AL, 'P'     ; 'P' for PASS
    INT 0x10        ; Call BIOS video service
    HLT             ; Halt the CPU 