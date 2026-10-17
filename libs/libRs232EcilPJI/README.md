# libRs232EcilPJI

libRs232EcilPJI communicates with the ECIL PJ-I (Jotta series) temperature
indicator over **RS-232C**, using the proprietary protocol in manual MO-02JRO-I
chapter 8. It is **not** Modbus.

This first version is **read-only**: each `get_all()` sends command **I**
(full parameter table) and converts the process variable to engineering units.
RS-422 command **A** (driver enable) is not used.

Serial settings are fixed by the instrument: **9600 baud, 8 data bits, even
parity, 1 stop bit** (8E1). Point-to-point RS-232C.

## Installing dependencies

This library uses libserialport.

Using ArchLinux family:

```Bash
sudo pacman -Sy libserialport
```

Using Debian family:

```Bash
sudo apt update && sudo apt install libserialport-dev
```

## Accessing serial ports

```Bash
sudo usermod -a -G dialout <your_user>
```

On Arch, the group is often `uucp` instead of `dialout`. Log out and back in.

On USB-serial adapters (FTDI), set the latency timer to 1 ms or the ~70 ms
reply to command I can be delayed or split:

```Bash
echo 1 | sudo tee /sys/bus/usb-serial/devices/ttyUSB0/latency_timer
```

## Instrument setup

On the PJ-I front panel, communication must be enabled and the address must
be 1–32 (00 takes the unit off the bus). Option **C2** is the RS-232C
interface. Remote write may stay disabled (computer read-only).

## Compiling just the lib

```Bash
g++ -c libRs232EcilPJI.cpp -lserialport -o libRs232EcilPJI.o
```

## Compiling the lib with test and running

```Bash
g++ test.cpp libRs232EcilPJI.cpp -lserialport -o testlibRs232EcilPJI.elf
./testlibRs232EcilPJI.elf /dev/ttyUSB0 1 10
```

Arguments: serial port, instrument address (1–32), number of reads.

```Bash
clear ; g++ test.cpp libRs232EcilPJI.cpp -lserialport -o testlibRs232EcilPJI.elf && ./testlibRs232EcilPJI.elf /dev/ttyUSB0 1 10
```

## License, Warranty and Copyright

This library comes with ABSOLUTELY NO WARRANTY; Is under GNU GENERAL PUBLIC LICENSE version 3 (GPL3), that means you ONLY can use in free software projects.
For more details read the file LICENSE.
Copyright (C) 2026  Thalles Campagnani
