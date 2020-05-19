import os from "os";
import path from "path";

if (os.platform() === "win32") {
    process.env.PATH = `${process.env.PATH};${path.resolve(__dirname, '../vendor/w2xc')};${path.resolve(__dirname, '../vendor/opencv/build/x64/vc15/bin')}`;
}

export const DEFAULT_MODELS_DIR = path.resolve(__dirname, '../vendor/w2xc/models_rgb');

export { default as Types } from "./types";
export { default as Enums } from "./enums";
export { default as W2XCJS } from "./constructor";