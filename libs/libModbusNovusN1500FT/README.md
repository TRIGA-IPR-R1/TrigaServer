# libModbusNovusN1500FT

libModbusNovusN1500FT communicates with the NOVUS N1500FT flow indicator over
**RS-485**, using **Modbus RTU** (protocol manual V1.0x B).

This first version is **read-only**. Each `get_all()` issues function **03h**
(Read Holding Registers) in four blocks:

| Registers | Contents |
|-----------|----------|
| 0–11 | Instant / total / grand-total / aux / max / min flow (IEEE-754) |
| 112–119 | Unit texts, time-base code, input type |
| 600–603 | Serial, firmware, product id (80) |
| 628–630 | Status words 1–3 |

Writes (05h / 06h / 16h) are not used.

Floats follow the instrument layout: **word high**, then **word low**, IEEE-754
single precision. Default serial settings: **9600 8N1**, slave address **1**.
Baud and parity are programmable on the indicator (1200–115200; N/E/O; 1 stop
bit) and can be passed to the constructor.

## Installing dependencies

This library uses libmodbus.

Using ArchLinux family:

```Bash
sudo pacman -Sy libmodbus
```

Using Debian family:

```Bash
sudo apt update && sudo apt install libmodbus-dev
```

## Accessing serial ports

```Bash
sudo usermod -a -G dialout <your_user>
```

On Arch, the group is often `uucp` instead of `dialout`. Log out and back in.

## Instrument setup

On the N1500FT, configure **bavd** (baud), **Parity** and **ADDR** (1–247) to
match the constructor. Product identifier register 603 must read `80`.

## Compiling just the lib

```Bash
g++ -c libModbusNovusN1500FT.cpp -lmodbus -o libModbusNovusN1500FT.o
```

## Compiling the lib with test and running

```Bash
g++ test.cpp libModbusNovusN1500FT.cpp -lmodbus -o testlibModbusNovusN1500FT.elf
./testlibModbusNovusN1500FT.elf /dev/ttyUSB0 1 10
```

Arguments: serial port, Modbus address (1–247), number of reads. Optional:
baud (default 9600) and parity `N|E|O` (default `N`).

```Bash
clear ; g++ test.cpp libModbusNovusN1500FT.cpp -lmodbus -o testlibModbusNovusN1500FT.elf && ./testlibModbusNovusN1500FT.elf /dev/ttyUSB0 1 10
```

## License, Warranty and Copyright

This library comes with ABSOLUTELY NO WARRANTY; Is under GNU GENERAL PUBLIC LICENSE version 3 (GPL3), that means you ONLY can use in free software projects.
For more details read the file LICENSE.
Copyright (C) 2026  Thalles Campagnani
