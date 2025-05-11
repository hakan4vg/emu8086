; Basic test program
; Using only instructions we know are supported

; Set up video mode (text mode 80x25)
MOV AX, 0x0003
INT 10h

; Test MOV instructions
MOV AX, 0x1234
MOV BX, 0x5678
MOV CX, AX      ; Register to register move

; Test ADD instruction
ADD AX, BX      ; AX = AX + BX (0x1234 + 0x5678 = 0x68AC)

; Test SUB instruction
SUB AX, 0x1000  ; AX = AX - 0x1000 (0x68AC - 0x1000 = 0x58AC)

; Test INC/DEC instructions
INC BX          ; BX = BX + 1 (0x5678 + 1 = 0x5679)
DEC CX          ; CX = CX - 1 (0x1234 - 1 = 0x1233)

; Display a character using INT 10h (teletype)
MOV AX, 0x0E41  ; AH=0x0E (teletype function), AL='A' (ASCII 0x41)
INT 10h         ; Display 'A'

; Display another character
MOV AX, 0x0E42  ; AH=0x0E (teletype function), AL='B' (ASCII 0x42)
INT 10h         ; Display 'B'

; Test JMP instruction
JMP next_section

; This should be skipped
HLT

next_section:
; Display another character to confirm jump worked
MOV AX, 0x0E43  ; AH=0x0E (teletype function), AL='C' (ASCII 0x43)
INT 10h         ; Display 'C'

; Newline
MOV AX, 0x0E0D  ; CR
INT 10h
MOV AX, 0x0E0A  ; LF
INT 10h

; End with halt instruction
HLT 