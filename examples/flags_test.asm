; Test program for flag manipulation instructions
; Tests CLC, STC, CMC, CLD, STD, CLI, STI
; Outputs flag status via INT 10h

; Setup
MOV AX, 0x0003    ; Text mode 80x25
INT 10h

; Display a character
DISPLAY_CHAR:
    MOV AH, 0x0E  ; Teletype output
    INT 10h
    RET

; Display newline
NEWLINE:
    PUSH AX
    MOV AL, 0x0D  ; CR
    CALL DISPLAY_CHAR
    MOV AL, 0x0A  ; LF
    CALL DISPLAY_CHAR
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
    CALL DISPLAY_CHAR
    INC SI            ; Next character
    JMP DISPLAY_LOOP
    
DISPLAY_DONE:
    POP SI
    POP AX
    RET

; Display YES/NO based on flag condition
DISPLAY_FLAG:
    JNC FLAG_NOT_SET
    MOV SI, MSG_YES
    JMP DISPLAY_FLAG_RESULT
FLAG_NOT_SET:
    MOV SI, MSG_NO
DISPLAY_FLAG_RESULT:
    CALL DISPLAY_MSG
    RET

; Test data
MSG_HEADER  DB 'Flag Manipulation Test', 0
MSG_CLC     DB 'CLC: Carry Flag Cleared - CF=', 0
MSG_STC     DB 'STC: Carry Flag Set - CF=', 0
MSG_CMC1    DB 'CMC (after STC): Carry Flag Complemented - CF=', 0
MSG_CMC2    DB 'CMC (after CMC): Carry Flag Complemented - CF=', 0
MSG_CLD     DB 'CLD: Direction Flag Cleared - DF=', 0
MSG_STD     DB 'STD: Direction Flag Set - DF=', 0
MSG_CLI     DB 'CLI: Interrupt Flag Cleared - IF=', 0
MSG_STI     DB 'STI: Interrupt Flag Set - IF=', 0
MSG_YES     DB 'YES', 0
MSG_NO      DB 'NO', 0

; Display header
MOV SI, MSG_HEADER
CALL DISPLAY_MSG
CALL NEWLINE
CALL NEWLINE

; Test CLC (Clear Carry Flag)
CLC                     ; Clear carry flag
MOV SI, MSG_CLC
CALL DISPLAY_MSG
CALL DISPLAY_FLAG       ; Display flag status
CALL NEWLINE

; Test STC (Set Carry Flag)
STC                     ; Set carry flag
MOV SI, MSG_STC
CALL DISPLAY_MSG
CALL DISPLAY_FLAG       ; Display flag status
CALL NEWLINE

; Test CMC (Complement Carry Flag) after STC
CMC                     ; Complement carry flag (should clear it)
MOV SI, MSG_CMC1
CALL DISPLAY_MSG
CALL DISPLAY_FLAG       ; Display flag status
CALL NEWLINE

; Test CMC (Complement Carry Flag) again
CMC                     ; Complement carry flag again (should set it)
MOV SI, MSG_CMC2
CALL DISPLAY_MSG
CALL DISPLAY_FLAG       ; Display flag status
CALL NEWLINE

; Test CLD (Clear Direction Flag)
CLD                     ; Clear direction flag
MOV SI, MSG_CLD
CALL DISPLAY_MSG

; Display Direction Flag status (need to use PUSHF/POPF)
PUSHF                   ; Push flags to stack
POP AX                  ; Pop into AX
TEST AX, 0x0400         ; Test DF bit (bit 10)
JNZ DF_SET
MOV SI, MSG_NO
JMP PRINT_DF
DF_SET:
MOV SI, MSG_YES
PRINT_DF:
CALL DISPLAY_MSG
CALL NEWLINE

; Test STD (Set Direction Flag)
STD                     ; Set direction flag
MOV SI, MSG_STD
CALL DISPLAY_MSG

; Display Direction Flag status (need to use PUSHF/POPF)
PUSHF                   ; Push flags to stack
POP AX                  ; Pop into AX
TEST AX, 0x0400         ; Test DF bit (bit 10)
JNZ DF_SET2
MOV SI, MSG_NO
JMP PRINT_DF2
DF_SET2:
MOV SI, MSG_YES
PRINT_DF2:
CALL DISPLAY_MSG
CALL NEWLINE

; Test CLI (Clear Interrupt Flag)
CLI                     ; Clear interrupt flag
MOV SI, MSG_CLI
CALL DISPLAY_MSG

; Display Interrupt Flag status (need to use PUSHF/POPF)
PUSHF                   ; Push flags to stack
POP AX                  ; Pop into AX
TEST AX, 0x0200         ; Test IF bit (bit 9)
JNZ IF_SET
MOV SI, MSG_NO
JMP PRINT_IF
IF_SET:
MOV SI, MSG_YES
PRINT_IF:
CALL DISPLAY_MSG
CALL NEWLINE

; Test STI (Set Interrupt Flag)
STI                     ; Set interrupt flag
MOV SI, MSG_STI
CALL DISPLAY_MSG

; Display Interrupt Flag status (need to use PUSHF/POPF)
PUSHF                   ; Push flags to stack
POP AX                  ; Pop into AX
TEST AX, 0x0200         ; Test IF bit (bit 9)
JNZ IF_SET2
MOV SI, MSG_NO
JMP PRINT_IF2
IF_SET2:
MOV SI, MSG_YES
PRINT_IF2:
CALL DISPLAY_MSG
CALL NEWLINE

; End program
HLT 