# Chip8 Emulator

## Usage

### Emulator
```./emulator [rom_path] --log --debug```

- `--log` enable debug (NCurses / printf, depending on NCURSES_LOGGING definition)
- `--debug` enable step-by-step instructions
  - left: backward
  - right: forward
  - space: fast-forward/backward (in combination with left/right)

### Assembler

`./assembler pong.asm PONG`

### Disassembler
`./disassembler PONG pong.asm`

### Keyboard
- Chip8's keyboard:

|||||
|---|---|---|---|
|1|2|3|C|
|4|5|6|D|
|7|8|9|E|
|A|0|B|F|

- qwerty equivalent:

|||||
|---|---|---|---|
|1|2|3|4|
|Q|W|E|R|
|A|S|D|F|
|Z|X|C|V|
