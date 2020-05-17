# Waifu2x Converter for NodeJS

A node addon module which wraps around [libw2xc from waifu2x-converter-cpp](https://github.com/DeadSix27/waifu2x-converter-cpp)

Used to upscale photos or Anime-style art using convolutional neural networks.

# Usage

This module currently only supports GNU/Linux, Windows support will come soon.

## Prerequisites

This project requires node-gyp to build, make sure it is installed using ```npm install -g node-gyp```

### Windows x64

Make sure you have node-gyp setup correctly. You'll need Visual Studio 2015 or later installed to compile the source. See https://www.npmjs.com/package/node-gyp#on-windows for more info.

Dependencies are installed automatically but require 7z to extract the binaries, make sure it is installed at the default install path `C:\Program Files\7-Zip\7z.exe`

The install scripts should install all dependencies automatically so no additional setup is required.

### Linux

Install the dependencies listed below.

Make sure the you install it in one of the linker's search directories. It should be by default if you use your package manager or follow the build instructions below.

#### OpenCV

Install OpenCV using your distrobution's package manager.

On arch you'll use ```pacman -S opencv```

#### waifu2x-converter-cpp

- AUR (ArchLinux based distros)
    - [waifu2x-converter-cpp-git](https://aur.archlinux.org/packages/waifu2x-converter-cpp-git/)

- Fedora
    - [waifu2x-converter-cpp](https://apps.fedoraproject.org/packages/waifu2x-converter-cpp)

- Other Linux
    - Build from source. See instructions here https://github.com/DeadSix27/waifu2x-converter-cpp/blob/master/BUILDING.md

# Documentation

Documentation is generated using TypeDoc, run `npm run docs:build` to build the documentation and `npm run docs:serve` to serve a local copy of the documentation.