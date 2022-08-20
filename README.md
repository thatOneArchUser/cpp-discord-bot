# cpp-discord-bot

## Installation
debian:
```bash
apt-get install wget
wget -0 dpp.deb https://dl.dpp.dev/
dpkg -i dpp.deb
```

arch (aur):
```bash
git clone https://aur.archlinux.org/dpp.git
cd dpp
makepkg -si

#or using yay
yay -Sy dpp
```

redhat, cent os:
```bash
yum install wget
wget -O dpp.rpm https://dl.dpp.dev/latest/linux-x64/rpm
yum localinstall dpp.rpm
```

vcpkg (compatible with all os):
```bash
vcpkg install dpp:x64-windows #replace x64-windows with whichever os and architecture you want the library to be built for
vcpkg integrate install
```

## Setup
just put your user id and bot token in settings.json

## Build
g++ needs to be installed

`g++ -std=c++17 -o bot main.cpp -ldpp`
