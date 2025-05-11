; Simple ADC/SBB test program

MOV AX, 0x0003    ; Text mode 80x25
INT 10h

; Test ADC without carry
CLC                ; Clear carry flag
MOV AX, 0x1234    ; First operand
MOV BX, 0x5678    ; Second operand
ADC AX, BX        ; AX = AX + BX + CF (0)

; Display 'A' to indicate ADC test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'A'        ; 'A' character
INT 10h           ; Display 'A'

; Test ADC with carry
STC                ; Set carry flag
MOV AX, 0x1234    ; First operand
MOV BX, 0x5678    ; Second operand
ADC AX, BX        ; AX = AX + BX + CF (1)

; Display 'C' to indicate ADC with carry test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'C'        ; 'C' character
INT 10h           ; Display 'C'

; Test SBB without borrow
CLC                ; Clear carry flag
MOV AX, 0x9999    ; First operand
MOV BX, 0x1111    ; Second operand
SBB AX, BX        ; AX = AX - BX - CF (0)

; Display 'S' to indicate SBB test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'S'        ; 'S' character
INT 10h           ; Display 'S'

; Test SBB with borrow
STC                ; Set carry flag
MOV AX, 0x9999    ; First operand
MOV BX, 0x1111    ; Second operand
SBB AX, BX        ; AX = AX - BX - CF (1)

; Display 'B' to indicate SBB with borrow test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'B'        ; 'B' character
INT 10h           ; Display 'B'

; Display newline
MOV AX, 0x0E0D    ; CR
INT 10h
MOV AX, 0x0E0A    ; LF
INT 10h

HLT               ; Halt execution 