; Simple test for JZ instruction
    MOV CX, 0
    JZ  label
    MOV AX, 1
label:
    HLT 