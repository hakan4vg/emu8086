; Simple test program for new instructions
; This tests various instructions added to the emulator

    ; Set up video mode
    MOV AX, 0x0003  ; AH=0 (set video mode), AL=3 (80x25 text)
    INT 0x10
    
    ; Test basic arithmetic
    MOV AX, 5       ; Load 5 into AX
    MOV BX, 3       ; Load 3 into BX
    
    ; Addition
    ADD AX, BX      ; AX = 5 + 3 = 8
    
    ; Subtraction
    SUB AX, 1       ; AX = 8 - 1 = 7
    
    ; Comparison and jumping
    CMP AX, 7       ; Compare AX with 7
    JNE not_equal   ; Jump if not equal
    
    ; Test shift and rotate operations
    MOV CX, 0xA55A  ; Test pattern 1010 0101 0101 1010
    
    ; Shifts
    SHL CX, 1       ; Shift left by 1 bit
    SHR CX, 1       ; Shift right by 1 bit
    
    ; Rotates
    MOV DX, 0x1234
    ROL DX, 4       ; Rotate left by 4 bits (should be 0x2341)
    ROR DX, 8       ; Rotate right by 8 bits (should be 0x4123)
    
    ; Carry flag operations
    CLC             ; Clear carry flag
    STC             ; Set carry flag
    CMC             ; Complement carry flag
    
    ; Exit
    MOV AH, 0x0E
    MOV AL, 'P'     ; Print 'P' for PASS
    INT 0x10
    HLT
    
not_equal:
    MOV AH, 0x0E
    MOV AL, 'F'     ; Print 'F' for FAIL
    INT 0x10
    HLT 