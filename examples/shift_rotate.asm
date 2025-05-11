; Test program for shift and rotate instructions
; Tests ROL, ROR, SHL/SAL, SHR, SAR, RCL, RCR on various data patterns
; Outputs results via INT 10h

; Setup
MOV AX, 0x0003    ; Text mode 80x25
INT 10h

; Initialize test value
MOV BX, 0xA55A    ; Test pattern 1010 0101 0101 1010

; Function to display a hex nibble (value in AL)
DISPLAY_NIBBLE:
    AND AL, 0x0F      ; Ensure only lower 4 bits
    ADD AL, '0'       ; Convert to ASCII
    CMP AL, '9'       ; Check if it's 0-9
    JLE PRINT_NIBBLE
    ADD AL, 7         ; Convert to A-F (7 = 'A'-'0'-10)
    
PRINT_NIBBLE:
    MOV AH, 0x0E      ; Teletype output
    INT 10h
    RET

; Function to display a 16-bit register value in hex (value in CX)
DISPLAY_REG:
    PUSH AX
    PUSH BX
    PUSH CX
    
    MOV BX, CX        ; Save value to display
    
    ; Display most significant byte
    MOV AL, BH
    MOV CL, 4
    SHR AL, CL        ; Get high nibble
    CALL DISPLAY_NIBBLE
    
    MOV AL, BH
    CALL DISPLAY_NIBBLE
    
    ; Display least significant byte
    MOV AL, BL
    MOV CL, 4
    SHR AL, CL        ; Get high nibble
    CALL DISPLAY_NIBBLE
    
    MOV AL, BL
    CALL DISPLAY_NIBBLE
    
    ; Display space
    MOV AL, ' '
    MOV AH, 0x0E
    INT 10h
    
    POP CX
    POP BX
    POP AX
    RET

; Display newline
NEWLINE:
    PUSH AX
    MOV AL, 0x0D      ; CR
    MOV AH, 0x0E
    INT 10h
    MOV AL, 0x0A      ; LF
    INT 10h
    POP AX
    RET

; Display a message (SI points to null-terminated string)
DISPLAY_MSG:
    PUSH AX
    PUSH SI
    
DISPLAY_LOOP:
    MOV AL, [SI]      ; Get character
    OR AL, AL         ; Check for null terminator
    JZ DISPLAY_DONE
    MOV AH, 0x0E      ; Teletype output
    INT 10h
    INC SI            ; Next character
    JMP DISPLAY_LOOP
    
DISPLAY_DONE:
    POP SI
    POP AX
    RET

; Test data
MSG_INIT DB 'Initial value: ', 0
MSG_ROL DB 'ROL by 4: ', 0
MSG_ROR DB 'ROR by 4: ', 0
MSG_SHL DB 'SHL by 2: ', 0
MSG_SHR DB 'SHR by 2: ', 0
MSG_SAR DB 'SAR by 2: ', 0
MSG_RCL DB 'RCL by 1: ', 0
MSG_RCR DB 'RCR by 1: ', 0
MSG_CFON DB 'With CF=1: ', 0
MSG_CFOFF DB 'With CF=0: ', 0

; Display initial value
MOV SI, MSG_INIT
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

; Test ROL (Rotate Left)
MOV SI, MSG_ROL
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
MOV CL, 4       ; Rotate by 4 bits
ROL BX, CL      ; Perform rotate
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test ROR (Rotate Right)
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_ROR
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
MOV CL, 4       ; Rotate by 4 bits
ROR BX, CL      ; Perform rotate
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test SHL (Shift Left Logical) - same as SAL
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_SHL
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
MOV CL, 2       ; Shift by 2 bits
SHL BX, CL      ; Perform shift
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test SHR (Shift Right Logical)
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_SHR
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
MOV CL, 2       ; Shift by 2 bits
SHR BX, CL      ; Perform shift
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test SAR (Shift Arithmetic Right) with a positive value
MOV BX, 0x5A5A  ; A positive value
MOV SI, MSG_SAR
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
MOV CL, 2       ; Shift by 2 bits
SAR BX, CL      ; Perform shift
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test SAR (Shift Arithmetic Right) with a negative value
MOV BX, 0xA55A  ; A negative value (bit 15 = 1)
MOV SI, MSG_SAR
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
MOV CL, 2       ; Shift by 2 bits
SAR BX, CL      ; Perform shift
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test RCL (Rotate through Carry Left) with CF=0
CLC             ; Clear carry flag
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_RCL
CALL DISPLAY_MSG
MOV SI, MSG_CFOFF
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
RCL BX, 1       ; Rotate through carry
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test RCL (Rotate through Carry Left) with CF=1
STC             ; Set carry flag
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_RCL
CALL DISPLAY_MSG
MOV SI, MSG_CFON
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
RCL BX, 1       ; Rotate through carry
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test RCR (Rotate through Carry Right) with CF=0
CLC             ; Clear carry flag
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_RCR
CALL DISPLAY_MSG
MOV SI, MSG_CFOFF
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
RCR BX, 1       ; Rotate through carry
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; Test RCR (Rotate through Carry Right) with CF=1
STC             ; Set carry flag
MOV BX, 0xA55A  ; Reset test value
MOV SI, MSG_RCR
CALL DISPLAY_MSG
MOV SI, MSG_CFON
CALL DISPLAY_MSG
MOV CX, BX      ; Original value
RCR BX, 1       ; Rotate through carry
MOV CX, BX      ; Display result
CALL DISPLAY_REG
CALL NEWLINE

; End program
HLT 