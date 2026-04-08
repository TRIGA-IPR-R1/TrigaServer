# TrigaServer

TrigaServer is a software for GNU operating system to get the real-time values of the Nuclear Reactor
Triga IPR-R1 and share in network.

## Compiling manually

### Installing dependencies

If you are using a Debian-based distribution, run:

``` Bash
sudo apt install git build-essential cmake libboost-all-dev libcxxopts-dev libmodbus-dev libserialport-dev libjsoncpp-dev
```

If you are using a Arch-based distribution, run:

``` Bash
sudo pacman -Syu git base-devel cmake boost cxxopts libserialport
paru -S libmodbus #AUR
```

### Clone and compiling

Afterwards, you can clone, build and compile with the following commands:

``` Bash
git clone https://github.com/TRIGA-IPR-R1/TrigaServer.git
cd TrigaServer
mkdir build
cd build
cmake ..
make
```

## Auto-Compiling and build a PKG

If you are using a Debian-based distribution, run:

``` Bash
git clone https://github.com/TRIGA-IPR-R1/TrigaServer.git
cd TrigaServer
cmake -B build
cmake --build build -j$(nproc)
cd build
cpack -G DEB
sudo apt install ./TrigaServer-*.deb
```

If you are using a Arch-based distribution, run:

``` Bash
paru -S libmodbus #AUR
mkdir -p ~/TrigaServer-PKG
cd ~/TrigaServer-PKG
wget https://raw.githubusercontent.com/TRIGA-IPR-R1/InstallGNU/ArchLinux/TrigaServer/PKGBUILD
makepkg -s #Or use -si to install when finish the build.
```

## License, Warranty and Copyright

This library comes with ABSOLUTELY NO WARRANTY; Is under GNU GENERAL PUBLIC LICENSE version 3
(GPL3), that means you ONLY can use in free software projects. For more details read the file
LICENSE. Copyright (C) 2023-2024 Thalles Campagnani
