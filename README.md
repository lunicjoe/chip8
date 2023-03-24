# Chip8 Emulator

```./chip8 [rom_path] --log --debug```

- `--log` enable debug (NCurses / printf, depending on NCURSES_LOGGING definition)
- `--debug` enable step-by-step instructions
  - left: backward
  - right: forward
  - space: fast-forward/backward (in combination with left/right)