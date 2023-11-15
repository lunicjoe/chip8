# Assembly

## Specifications

- Code is case-insensitive

- Comments
```asm
# lines who begin with '#' are comments
```

- Labels
```asm
# loop example:
:loop
add V0, 1
jmp $loop
```

- Data
```asm
# following lines will be saved each ones as 16 bits values in the rom
0x12ef
0o1278
0b10
1289
```

- Include

```asm
%include file.asm
```

## Chip8 Instructions

- Clear the screen
```asm
cls
```

- Return from a subroutine
```asm
ret
```

- Jump to a specific address
```asm
jmp 0x300
jmp $label
```

- Call a subroutine
```asm
call 0x300
call $label
```

- Skip if equal
```asm
se VX, VY
se VX, 123
```

- Skip if not equal
```asm
sne VX, VY
sne VX, 123
```

- Store a value into a specified register
```asm
ld VX, VY
# store from V0 to VX in memory at address I
ld [I], VX
# fills from V0 to VX from memory starting at address I
ld VX, [I]
# store delay timer
ld VX, DT
# store pressed key
ld VX, KEY
```

- Add value to a specified register
```asm
add VX, VY
add VX, 123
add I, VX
```

- Store random number into VX register with a specified mask
```asm
rnd VX, 0x0f
```

- Draw sprite stored at the index register at VX, VY position with 0xf rows
```asm
drw VX, VY, 0xf
```

- Stores the binary-coded decimal representation of VX with digit
stored in memory at location in I, I+1, I+2.
```asm
bcd VX
```

- Skip next instruction if VX key pressed.
```asm
skp VX
```

- Skip next instruction if VX key is not pressed.
```asm
sknp VX
```

- Arithmetic operations
```asm
or VX, VY
and VX, VY
xor VX, VY
sub VX, VY
shr VX
subn VX, VY
shl VX
```

- Store the location of the sprite corresponding to the VX character
```asm
font VX
```

# Rom example
```asm
jmp $game

:player
0b0011100000101000
0b0011100000010000
0b1111111000010000
0b0010100000101000
0b0100010001000100
0b0100010000000000

:game
ld I, $player
ld VE, 1

# game loop
:loop
cls
drw V0, V1, 11
ld VA, KEY
sne VA, 0X5
call $up
sne VA, 0X8
call $down
sne VA, 0X7
call $left
sne VA, 0X9
call $right
jmp $loop

# game movements
:up
sub V1, VE
ret

:down
add V1, 1
ret

:left
sub V0, VE
ret

:right
add V0, 1
ret
```