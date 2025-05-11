; Simple shift/rotate test
MOV CX, 0x1234
ROL CX, 1      ; Rotate left by 1
ROR CX, 1      ; Rotate right by 1
SHL CX, 1      ; Shift left by 1
SHR CX, 1      ; Shift right by 1
SAR CX, 1      ; Shift arithmetic right by 1
HLT 