# AmigaZen Scientific Calculator (SciCalc)

A scientific calculator for the Amiga computer system with advanced mathematical functions.

## Features

- Scientific functions (sin, cos, tan, log, ln, etc.)
- Memory registers
- Multiple bases (decimal, hexadecimal, octal, binary)
- Trigonometric modes (degrees, radians)
- Commodities support for hotkeys
- Clipboard integration

## Building

This project requires SAS/C compiler to build:

1. Make sure SAS/C compiler is installed and in your path
2. Run `smake` to build the project
3. Run `smake install` to copy to SYS:Utilities

## Command Line Arguments

```
SciCalc PUBSCREEN/K,TAPE/K,MEMORY/N
```

- PUBSCREEN - Name of public screen to open on
- TAPE - Path to tape file for logging calculations
- MEMORY - Number of memory registers (default: 10)

## Workbench Tool Types

- PUBSCREEN - Public screen to open on
- TAPE - Path to tape file
- MEMORY - Number of memory registers
- CX_POPKEY - Commodities hotkey to show calculator

## License

Copyright © 2024 AmigaZen.com
