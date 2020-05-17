import Enums from "./enums";
import Types from "./types";

declare class W2XCJS {
    /**
     * Create a waif2x converter instance
     * @param gpu The processor type to use.
     * @param njob Leave as 0 for auto or look at waifu2x-converter-cpp source.
     * @param log_level Log progress to console, value from 0 to 4.
     */
    constructor (gpu : Enums.W2XConvGPUMode = Enums.W2XConvGPUMode.W2XCONV_GPU_AUTO, njob : number = 0, log_level = 0);

    /**
     * @returns
     * Returns info regarding the converter
     */
    public getConv() : Types.W2XConv;

    /**
     * Set the models directory
     * @remarks
     * You must call this function before trying to convert a file else it will throw an error
     * @param model_dir The path to the models direcotory
     * @returns
     * Returns 0 if successfull.
     */
    public loadModels(model_dir: string) : number;

    /**
     * Process file using waifu2x.
     * @param src_path Path to the source image
     * @param dst_path Path to the destination image
     * @param denoise_level From w2xconv.h: -1:none, 0:L0 denoise, 1:L1 denoise, 2:L2 denoise, 3:L3 denoise
     * @param scale Scale factor
     * @param imwrite_params Parameters used by opencv to save the image.
     * @returns
     * Returns 0 if successful.
     */
    public convertFile(src_path: string, dst_path: string, denoise_level: number = -1, scale: number = 2.0, imwrite_params: Types.W2XJSImwriteParams = { webp_quality: 101, jpeg_quality: 90, png_compression: 5 }) : number;

    /**
     * Process image buffer using waifu2x.
     * @remarks
     * This function is only available natively on the linux platform. It works on Windows via a hacky pollyfill.
     * @param src_buffer The buffer containing the image to convert
     * @param dst_ext The file extension of the resulting image
     * @param denoise_level From w2xconv.h: -1:none, 0:L0 denoise, 1:L1 denoise, 2:L2 denoise, 3:L3 denoise
     * @param scale Scale factor
     * @param imwrite_params Parameters used by opencv to save the image.
     * @returns
     * The encoded image buffer.
     */
    public convertBuffer(src_buffer: Buffer, dst_ext: string, denoise_level: number = -1, scale: number = 2.0, imwrite_params: Types.W2XJSImwriteParams = { webp_quality: 101, jpeg_quality: 90, png_compression: 5 }) : Buffer;
}

export default W2XCJS;