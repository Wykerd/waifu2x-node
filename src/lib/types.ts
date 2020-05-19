import Enums from "./enums";

namespace Types {
    export interface W2XConvError {
        code: Enums.W2XConvErrorCode;
    }

    export interface W2XConvFlopsCounter {
        flop: number;
        filter_sec: number;
        process_sec: number;
    }

    export interface W2XConvProcessor {
        type: Enums.W2XConvProcessorType;
        sub_type: Enums.W2XConvProcessorSubType;
        dev_id: number;
        num_core: number;
        dev_name: string;
    }

    export interface W2XConv {
        last_error: W2XConvError;
        flops: W2XConvFlopsCounter;
        target_processor: W2XConvProcessor;
        log_level: number;
        tta_mode: boolean;
    }
    
    export interface ImwriteParams {
        webp_quality?: number;
        jpeg_quality?: number;
        png_compression?: number;
    }

    export type DenoiseLevel = -1 | 0 | 1 | 2 | 3;

    export interface AsyncOptions {
        imwrite_params: ImwriteParams;
        denoise_level: DenoiseLevel;
        scale: number;
    }

    export type ConvertCallback = (dst: Buffer) => any;
}

export default Types;