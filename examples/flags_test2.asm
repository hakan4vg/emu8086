; Simple flags manipulation test program

MOV AX, 0x0003    ; Text mode 80x25
INT 10h

; Test CLC (Clear Carry Flag)
STC                ; Set carry flag to 1
CLC                ; Clear carry flag to 0

; Display 'L' to indicate CLC test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'L'        ; 'L' character
INT 10h           ; Display 'L'

; Test STC (Set Carry Flag)
CLC                ; Clear carry flag to 0
STC                ; Set carry flag to 1

; Display 'S' to indicate STC test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'S'        ; 'S' character
INT 10h           ; Display 'S'

; Test CMC (Complement Carry Flag)
CLC                ; Clear carry flag to 0
CMC                ; Complement carry flag (0->1)

; Display '1' to indicate first CMC test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, '1'        ; '1' character
INT 10h           ; Display '1'

CMC                ; Complement carry flag again (1->0)

; Display '0' to indicate second CMC test
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, '0'        ; '0' character
INT 10h           ; Display '0'

; Test CLD (Clear Direction Flag)
STD                ; Set direction flag to 1
CLD                ; Clear direction flag to 0

; Display 'D' to indicate direction flag tests
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'D'        ; 'D' character
INT 10h           ; Display 'D'

; Test STD (Set Direction Flag)
CLD                ; Clear direction flag to 0
STD                ; Set direction flag to 1

; Display 'F' to indicate direction flag tests
MOV AX, 0x0E00    ; Teletype function for INT 10h
OR AL, 'F'        ; 'F' character
INT 10h           ; Display 'F'

; Newline
MOV AX, 0x0E0D    ; CR
INT 10h
MOV AX, 0x0E0A    ; LF
INT 10h

HLT               ; Halt execution 