; Simple Hello World for emu8086
; Direct character output using INT 10h, function 0Eh

    ; Set up video mode
    MOV AX, 0x0003  ; AH=0 (set video mode), AL=3 (80x25 text)
    INT 0x10
    
    ; Print 'H'
    MOV AX, 0x0E48  ; AH=0E (teletype), AL='H'
    INT 0x10
    
    ; Print 'e'
    MOV AX, 0x0E65  ; AH=0E (teletype), AL='e' 
    INT 0x10
    
    ; Print 'l'
    MOV AX, 0x0E6C  ; AH=0E (teletype), AL='l'
    INT 0x10
    
    ; Print 'l'
    MOV AX, 0x0E6C  ; AH=0E (teletype), AL='l'
    INT 0x10
    
    ; Print 'o'
    MOV AX, 0x0E6F  ; AH=0E (teletype), AL='o'
    INT 0x10
    
    ; Print '!'
    MOV AX, 0x0E21  ; AH=0E (teletype), AL='!'
    INT 0x10
    
    ; Halt the CPU
    HLT 