import os from "os";
import path from "path";

if (os.platform() === "win32") {
    process.env.PATH = `${process.env.PATH};${path.resolve(__dirname, 'vendor')}`;
}

export const MODELS_DIR = path.resolve(__dirname, 'vendor/models_rgb');

export { default as Types } from "./types";
export { default as Enums } from "./enums";
export { default as W2XCJS } from "./constructor";