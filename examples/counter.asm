; Counter Example for emu8086
; This example counts from 1 to 5 and displays the digits

    ; Set video mode
    MOV AX, 0x0003  ; AH=0 (set video mode), AL=3 (80x25 text)
    INT 0x10
    
    ; Print '1'
    MOV AX, 0x0E31  ; AH=0E (teletype), AL='1'
    INT 0x10
    
    ; Print '2'
    MOV AX, 0x0E32  ; AH=0E (teletype), AL='2'
    INT 0x10
    
    ; Print '3'
    MOV AX, 0x0E33  ; AH=0E (teletype), AL='3'
    INT 0x10
    
    ; Print '4'
    MOV AX, 0x0E34  ; AH=0E (teletype), AL='4'
    INT 0x10
    
    ; Print '5'
    MOV AX, 0x0E35  ; AH=0E (teletype), AL='5'
    INT 0x10
    
    ; Halt the CPU
    HLT 