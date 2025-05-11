; Shift operations test
; Tests shift and rotate operations

    ; Set up video mode
    MOV AX, 0x0003  ; AH=0 (set video mode), AL=3 (80x25 text)
    INT 0x10
    
    ; Test shift operations with immediates
    MOV CX, 0x1234
    SHL CX, 1       ; CX = 0x2468
    SHR CX, 1       ; CX = 0x1234 again
    ROL CX, 1       ; Rotate left by 1
    ROR CX, 1       ; Rotate right by 1
    SAR CX, 1        ; Shift arithmetic right by 1
    
    ; Test output
    MOV AH, 0x0E    ; Teletype output function
    MOV AL, 'S'     ; 'S' for Shift test
    INT 0x10
    HLT 