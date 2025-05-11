; Test program for ADC (Add with Carry) and SBB (Subtract with Borrow)
; Tests with various combinations of values and carry flag states
; Outputs results via INT 10h

; Setup
MOV AX, 0x0003    ; Text mode 80x25
INT 10h

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
MSG_HEADER  DB 'ADC/SBB Instruction Tests', 0
MSG_ADC     DB 'ADC (Add with Carry) Tests:', 0
MSG_SBB     DB 'SBB (Subtract with Borrow) Tests:', 0
MSG_VAL1    DB 'Value 1: ', 0
MSG_VAL2    DB 'Value 2: ', 0
MSG_CF_ON   DB 'With CF=1: ', 0
MSG_CF_OFF  DB 'With CF=0: ', 0
MSG_RESULT  DB 'Result: ', 0
MSG_FLAGS   DB 'Flags: ', 0
MSG_OF      DB 'OF=', 0
MSG_CF      DB ' CF=', 0
MSG_ZF      DB ' ZF=', 0
MSG_SF      DB ' SF=', 0
MSG_YES     DB 'Y', 0
MSG_NO      DB 'N', 0

; Display flags status (must be called right after the operation)
DISPLAY_FLAGS:
    PUSH AX
    PUSH SI
    
    MOV SI, MSG_FLAGS
    CALL DISPLAY_MSG
    
    ; Display Overflow Flag (OF)
    MOV SI, MSG_OF
    CALL DISPLAY_MSG
    PUSHF
    POP AX
    TEST AX, 0x0800   ; Test OF bit (bit 11)
    JZ OF_CLEAR
    MOV SI, MSG_YES
    JMP OF_DISPLAY
OF_CLEAR:
    MOV SI, MSG_NO
OF_DISPLAY:
    CALL DISPLAY_MSG
    
    ; Display Carry Flag (CF)
    MOV SI, MSG_CF
    CALL DISPLAY_MSG
    PUSHF
    POP AX
    TEST AX, 0x0001   ; Test CF bit (bit 0)
    JZ CF_CLEAR
    MOV SI, MSG_YES
    JMP CF_DISPLAY
CF_CLEAR:
    MOV SI, MSG_NO
CF_DISPLAY:
    CALL DISPLAY_MSG
    
    ; Display Zero Flag (ZF)
    MOV SI, MSG_ZF
    CALL DISPLAY_MSG
    PUSHF
    POP AX
    TEST AX, 0x0040   ; Test ZF bit (bit 6)
    JZ ZF_CLEAR
    MOV SI, MSG_YES
    JMP ZF_DISPLAY
ZF_CLEAR:
    MOV SI, MSG_NO
ZF_DISPLAY:
    CALL DISPLAY_MSG
    
    ; Display Sign Flag (SF)
    MOV SI, MSG_SF
    CALL DISPLAY_MSG
    PUSHF
    POP AX
    TEST AX, 0x0080   ; Test SF bit (bit 7)
    JZ SF_CLEAR
    MOV SI, MSG_YES
    JMP SF_DISPLAY
SF_CLEAR:
    MOV SI, MSG_NO
SF_DISPLAY:
    CALL DISPLAY_MSG
    
    POP SI
    POP AX
    RET

; Display header
MOV SI, MSG_HEADER
CALL DISPLAY_MSG
CALL NEWLINE
CALL NEWLINE

; ----- ADC Tests -----
MOV SI, MSG_ADC
CALL DISPLAY_MSG
CALL NEWLINE

; Test 1: ADC with normal values, CF=0
CLC             ; Clear carry flag
MOV AX, 0x1234  ; First operand
MOV BX, 0x5678  ; Second operand

; Display values
MOV SI, MSG_VAL1
CALL DISPLAY_MSG
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_VAL2
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_CF_OFF
CALL DISPLAY_MSG
ADC AX, BX      ; AX = AX + BX + CF (CF=0)
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE
CALL DISPLAY_FLAGS
CALL NEWLINE
CALL NEWLINE

; Test 2: ADC with normal values, CF=1
MOV AX, 0x1234  ; First operand
MOV BX, 0x5678  ; Second operand
STC             ; Set carry flag

; Display values
MOV SI, MSG_VAL1
CALL DISPLAY_MSG
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_VAL2
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_CF_ON
CALL DISPLAY_MSG
ADC AX, BX      ; AX = AX + BX + CF (CF=1)
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE
CALL DISPLAY_FLAGS
CALL NEWLINE
CALL NEWLINE

; Test 3: ADC with carry/overflow
MOV AX, 0xFFFF  ; First operand (maximum value)
MOV BX, 0x0001  ; Second operand
STC             ; Set carry flag

; Display values
MOV SI, MSG_VAL1
CALL DISPLAY_MSG
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_VAL2
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_CF_ON
CALL DISPLAY_MSG
ADC AX, BX      ; Should give 0x0001 with CF=1 (overflow)
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE
CALL DISPLAY_FLAGS
CALL NEWLINE
CALL NEWLINE

; ----- SBB Tests -----
MOV SI, MSG_SBB
CALL DISPLAY_MSG
CALL NEWLINE

; Test 1: SBB with normal values, CF=0
CLC             ; Clear carry flag
MOV AX, 0x5678  ; First operand
MOV BX, 0x1234  ; Second operand

; Display values
MOV SI, MSG_VAL1
CALL DISPLAY_MSG
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_VAL2
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_CF_OFF
CALL DISPLAY_MSG
SBB AX, BX      ; AX = AX - BX - CF (CF=0)
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE
CALL DISPLAY_FLAGS
CALL NEWLINE
CALL NEWLINE

; Test 2: SBB with normal values, CF=1
MOV AX, 0x5678  ; First operand
MOV BX, 0x1234  ; Second operand
STC             ; Set carry flag

; Display values
MOV SI, MSG_VAL1
CALL DISPLAY_MSG
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_VAL2
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_CF_ON
CALL DISPLAY_MSG
SBB AX, BX      ; AX = AX - BX - CF (CF=1)
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE
CALL DISPLAY_FLAGS
CALL NEWLINE
CALL NEWLINE

; Test 3: SBB with borrow (negative result)
MOV AX, 0x1234  ; First operand
MOV BX, 0x5678  ; Second operand
CLC             ; Clear carry flag

; Display values
MOV SI, MSG_VAL1
CALL DISPLAY_MSG
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_VAL2
CALL DISPLAY_MSG
MOV CX, BX
CALL DISPLAY_REG
CALL NEWLINE

MOV SI, MSG_CF_OFF
CALL DISPLAY_MSG
SBB AX, BX      ; Should give negative result with CF=1 (borrow)
MOV CX, AX
CALL DISPLAY_REG
CALL NEWLINE
CALL DISPLAY_FLAGS
CALL NEWLINE

; End program
HLT 