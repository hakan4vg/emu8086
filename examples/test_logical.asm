; Simple logical operation test
MOV AX, 0x1234
MOV BX, 0x5678
AND AX, BX      ; Logical AND
OR  AX, BX      ; Logical OR
XOR AX, BX      ; Logical XOR
AND AX, 0x00FF  ; AND with immediate
OR  AX, 0xFF00  ; OR with immediate
XOR AX, 0xFFFF  ; XOR with immediate
HLT 