; Simple shift/rotate test

MOV AX, 0x0003    ; Text mode 80x25
INT 10h

; Test SHL
MOV BX, 0x1234    ; Test value
SHL BX, 1         ; Shift left by 1
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'S'        ; 'S' character
INT 10h           ; Display 'S'

; Test SHR
MOV BX, 0x1234    ; Test value
SHR BX, 1         ; Shift right by 1
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'R'        ; 'R' character
INT 10h           ; Display 'R'

; Test ROL
MOV BX, 0x1234    ; Test value
ROL BX, 1         ; Rotate left by 1
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'L'        ; 'L' character
INT 10h           ; Display 'L'

; Test ROR
MOV BX, 0x1234    ; Test value
ROR BX, 1         ; Rotate right by 1 
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'O'        ; 'O' character
INT 10h           ; Display 'O'

; Test SAR
MOV BX, 0x8000    ; Test value with high bit set
SAR BX, 1         ; Shift arithmetic right by 1
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'A'        ; 'A' character
INT 10h           ; Display 'A'

; Test RCL
CLC                ; Clear carry flag
MOV BX, 0x1234     ; Test value
RCL BX, 1          ; Rotate through carry left by 1
MOV AX, 0x0E00     ; Teletype function for INT 10h
OR AL, 'C'         ; 'C' character
INT 10h            ; Display 'C'

; Test RCR
STC                ; Set carry flag
MOV BX, 0x1234     ; Test value
RCR BX, 1          ; Rotate through carry right by 1
MOV AX, 0x0E00     ; Teletype function for INT 10h
OR AL, 'D'         ; 'D' character
INT 10h            ; Display 'D'

HLT                ; Halt execution 