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
    
    export interface W2XJSImwriteParams {
        webp_quality?: number;
        jpeg_quality?: number;
        png_compression?: number;
    }
}

export default Types;