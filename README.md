# Waifu2x Converter for NodeJS

NodeJS bindings / wrapper for using [libw2xc from waifu2x-converter-cpp](https://github.com/DeadSix27/waifu2x-converter-cpp)

Used to upscale photos or Anime-style art using convolutional neural networks.

# Usage

This module currently only supports GNU/Linux and Windows.

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

## Installation

Install using npm
```
npm install waifu2x-node
```

## Synchronous Examples

### Upscaling a file

```typescript
import { W2XCJS, DEFAULT_MODELS_DIR } from 'waifu2x-node';

const converter = new W2XCJS();

const err = converter.loadModels(DEFAULT_MODELS_DIR);

if (!err) {
    const conv_err = converter.convertFile("in.png", "out.webp");
    if (!err) {
        console.log('File converted successfully');
    }
}
```

### Upscale a buffer

```typescript
import { W2XCJS, DEFAULT_MODELS_DIR } from 'waifu2x-node';
import fs from 'fs';

const converter = new W2XCJS();

const err = converter.loadModels(DEFAULT_MODELS_DIR);

if (!err) {
    const input_buffer = fs.readFileSync("in.png");
    const output_buffer = converter.convertBuffer(input_buffer, '.JPG'); // second parameter is the file extension to encode to.
    fs.writeFileSync("out.jpg", output_buffer);
}
```

## Asynchronous examples

Asynchronous functions only work on GPU processor types due to instabilities on the CPU

### Upscaling using callbacks

```typescript
import { W2XCJS, DEFAULT_MODELS_DIR } from 'waifu2x-node';
import fs from 'fs';

const converter = new W2XCJS();

const err = converter.loadModels(DEFAULT_MODELS_DIR); // model loading is synchronous

if (!err) {
    fs.readFile("in.png", (err, input_buffer) => {
        if (err) throw err;
        converter.convertBufferAsync(input_buffer, '.WEBP', { /* AsyncOptions */ }, dst_buffer => {
            fs.writeFile("out.webp", dst_buffer, err => {
                if (err) throw err;
            })
        })
    });
}
```

### Upscaling using promises

The library provides a wrapper class for using promises

```typescript
import { W2XCJS, DEFAULT_MODELS_DIR, W2XCJSPromises } from 'waifu2x-node';
import fs from 'fs';

const promises = new W2XCJSPromises(new W2XCJS());

const err = promises.converter.loadModels(DEFAULT_MODELS_DIR); // model loading is synchronous

if (!err) {
    (async () => {
        const input_buffer = await fs.promises.readFile("in.png");
        const dst_buffer = await promises.convertBuffer(input_buffer, '.WEBP', { /* AsyncOptions */ });
        await fs.promises.writeFile("out.webp", dst_buffer);
    })();
}
```

### Asynchronous convert options (AsyncOptions)

Abstract of the library source for reference, you could also generate the documentation for more detailed overview.

```typescript
interface AsyncOptions {
    // encoding options for destination buffer.
    imwrite_params: ImwriteParams;
    // denoising options (number value from -1 to 3 where -1 is no denoising)
    denoise_level: DenoiseLevel;
    // Scale factor.
    scale: number;
}
```

```typescript
interface ImwriteParams {
    // quality factor for webp and jpeg from 0 to 101 where 101 is lossless.
    webp_quality?: number;
    jpeg_quality?: number;
    // compression factor for png from 0 to 9 where 9 is smallest size and longest time.
    png_compression?: number;
}
```

# Documentation

Documentation is generated using TypeDoc, run `npm run docs:build` to build the documentation and `npm run docs:serve` to serve a local copy of the documentation.