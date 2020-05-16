# Waifu2x Converter for NodeJS

A node addon module which wraps around [libw2xc from waifu2x-converter-cpp](https://github.com/DeadSix27/waifu2x-converter-cpp)

Used to upscale photos or Anime-style art using convolutional neural networks.

# Usage

This module currently only supports GNU/Linux, Windows support will come soon.

## Prerequisites

### OpenCV

#### Linux

Install OpenCV using your distrobution's package manager.

On arch you'll use ```pacman -S opencv```

Make sure the install in one of the linker's search directories. It should be by default if you simply follow the build guide above or use a package.

### waifu2x-converter-cpp

#### Linux

- AUR (ArchLinux based distros)
    - [waifu2x-converter-cpp-git](https://aur.archlinux.org/packages/waifu2x-converter-cpp-git/)

- Fedora
    - [waifu2x-converter-cpp](https://apps.fedoraproject.org/packages/waifu2x-converter-cpp)

- Other Linux
    - Build from source. See instructions here https://github.com/DeadSix27/waifu2x-converter-cpp/blob/master/BUILDING.md

Make sure the install in one of the linker's search directories. It should be by default if you simply follow the build guide above or use a package.